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
//  File:	Blend.cc
//  Project:	XIL
//  Revision:	1.15
//  Last Mod:	10:10:35, 03/10/00
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
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)Blend.cc	1.15\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"

//
//  NOTE:  In the XIL_GENERAL implementations, these routines reference
//         ci.src3Data and other data members.  The ci data members are set
//         correctly to the information for the first band of the image.
//         Since Alpha is always a single banded image, there is no need to
//         reference more than the first band.
//


XilStatus
XilDeviceManagerComputeBYTE::Blenda1(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl)

{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    //
    //  Loop iterators.
    //
    unsigned int          x;
    unsigned int          y;
    unsigned int          o;

    while(ci.hasMoreInfo()) {
	if((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.src2Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)))
	{
            //
            //  The Alpha image.
            //
            Xil_unsigned8* srcA_scanline        = (Xil_unsigned8*) ci.src3Data;
            unsigned int   srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int   srcA_offset          = ci.src3Offset;

            //
            //  The input images.
            //
	    Xil_unsigned8* src1_scanline        = (Xil_unsigned8*) ci.src1Data;
            Xil_unsigned8* src2_scanline        = (Xil_unsigned8*) ci.src2Data;
            Xil_unsigned8* dest_scanline        = (Xil_unsigned8*) ci.destData;

            unsigned int   src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int   src2_scanline_stride = ci.src2ScanlineStride;
            unsigned int   dest_scanline_stride = ci.destScanlineStride;

            unsigned int   src1_pixel_stride    = ci.src1PixelStride;
            unsigned int   src2_pixel_stride    = ci.src2PixelStride;
            unsigned int   dest_pixel_stride    = ci.destPixelStride;

            if(nbands == 1) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_unsigned8* src1 = src1_scanline;
                    Xil_unsigned8* src2 = src2_scanline;
                    Xil_unsigned8* dest = dest_scanline;

                    for(x=0, o=x+srcA_offset; x<ci.xsize; x++, o++) {
                        if(XIL_BMAP_TST(srcA_scanline, o) == 0) {
                            *dest = *src1;
                        } else {
                            *dest = *src2;
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else if(nbands == 3) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_unsigned8* src1 = src1_scanline;
                    Xil_unsigned8* src2 = src2_scanline;
                    Xil_unsigned8* dest = dest_scanline;

                    for(x=0, o=x+srcA_offset; x<ci.xsize; x++, o++) {
                        if(XIL_BMAP_TST(srcA_scanline, o) == 0) {
                            *dest     = *src1;
                            *(dest+1) = *(src1+1);
                            *(dest+2) = *(src1+2);
                        } else {
                            *dest     = *src2;
                            *(dest+1) = *(src2+1);
                            *(dest+2) = *(src2+2);
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else {
                for(y=ci.ysize; y>0; y--) {
                    Xil_unsigned8* src1_pixel = src1_scanline;
                    Xil_unsigned8* src2_pixel = src2_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for(x=0, o=x+srcA_offset; x<ci.xsize; x++, o++) {
                        Xil_unsigned8* src1 = src1_pixel;
                        Xil_unsigned8* src2 = src2_pixel;
                        Xil_unsigned8* dest = dest_pixel;

                        if(XIL_BMAP_TST(srcA_scanline, o) == 0) {
                            for(unsigned int band=0; band<nbands; band++) {
                                *dest++ = *src1++;
                            }
                        } else {
                            for(unsigned int band=0; band<nbands; band++) {
                                *dest++ = *src2++;
                            }
                        }

                        src1_pixel += src1_pixel_stride;
                        src2_pixel += src2_pixel_stride;
                        dest_pixel += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	} else {
            //
            //  The Alpha image.
            //
            Xil_unsigned8* srcA_data            = (Xil_unsigned8*) ci.src3Data;
            unsigned int   srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int   srcA_offset          = ci.src3Offset;

	    for(unsigned int band=0; band<nbands; band++) { 
                //
                //  The input images.
                //
                Xil_unsigned8* src1_scanline        =
                    (Xil_unsigned8*)ci.getSrc1Data(band);
                Xil_unsigned8* src2_scanline        =
                    (Xil_unsigned8*)ci.getSrc2Data(band);
                Xil_unsigned8* dest_scanline        =
                    (Xil_unsigned8*)ci.getDestData(band);

                unsigned int   src1_scanline_stride =
                    ci.getSrc1ScanlineStride(band);
                unsigned int   src2_scanline_stride =
                    ci.getSrc2ScanlineStride(band);
                unsigned int   dest_scanline_stride =
                    ci.getDestScanlineStride(band);

                unsigned int   src1_pixel_stride    =
                    ci.getSrc1PixelStride(band); 
                unsigned int   src2_pixel_stride    =
                    ci.getSrc2PixelStride(band); 
                unsigned int   dest_pixel_stride    =
                    ci.getDestPixelStride(band); 

                Xil_unsigned8* srcA_scanline        = srcA_data;

                for(y=ci.ysize; y>0; y--) { 
                    Xil_unsigned8* src1 = src1_scanline;
                    Xil_unsigned8* src2 = src2_scanline;
                    Xil_unsigned8* dest = dest_scanline;

                    for(x=0, o=x+srcA_offset; x<ci.xsize; x++, o++) {
                        if(XIL_BMAP_TST(srcA_scanline, o) == 0) {
                            *dest = *src1;
                        } else {
                            *dest = *src2;
                        }

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	}
    }

    return ci.returnValue;
}


#define BLEND(a,b) \
    ((((unsigned int)(a)) + ((unsigned int)(b))) >> _XIL_BLEND_FRAC_BITS)

#ifdef XIL_LITTLE_ENDIAN
#define GET_FOURTH_BYTE(src)   (((src) >> 24) & 255)
#define GET_THIRD_BYTE(src)  (((src) >> 16) & 255)
#define GET_SECOND_BYTE(src)   (((src) >> 8) & 255)
#define GET_FIRST_BYTE(src)  ((src) & 255)
#else
#define GET_FIRST_BYTE(src)   (((src) >> 24) & 255)
#define GET_SECOND_BYTE(src)  (((src) >> 16) & 255)
#define GET_THIRD_BYTE(src)   (((src) >> 8) & 255)
#define GET_FOURTH_BYTE(src)  ((src) & 255)
#endif

XilStatus
XilDeviceManagerComputeBYTE::Blenda8(XilOp*       op,
				     unsigned     op_count,
				     XilRoi*      roi,
				     XilBoxList*  bl)

{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    if(getBlendTable(ci.getSystemState()) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    
    unsigned int nbands = ci.destNumBands;

    while(ci.hasMoreInfo()) {
	unsigned int x;
        unsigned int y;
        unsigned int band;
        BlendArray   blend_table = blendTable;

	if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {
            //
            //  The Alpha image.
            //
            Xil_unsigned8* srcA_scanline        = (Xil_unsigned8*) ci.src3Data;
            unsigned int   srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int   srcA_pixel_stride    = ci.src3PixelStride;

            //
            //  The input images.
            //
	    Xil_unsigned8* src1_scanline        = (Xil_unsigned8*) ci.src1Data;
            Xil_unsigned8* src2_scanline        = (Xil_unsigned8*) ci.src2Data;
            Xil_unsigned8* dest_scanline        = (Xil_unsigned8*) ci.destData;

            unsigned int   src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int   src2_scanline_stride = ci.src2ScanlineStride;
            unsigned int   dest_scanline_stride = ci.destScanlineStride;

            unsigned int   src1_pixel_stride    = ci.src1PixelStride;
            unsigned int   src2_pixel_stride    = ci.src2PixelStride;
            unsigned int   dest_pixel_stride    = ci.destPixelStride;

	    if((nbands            == 1) &&
               (dest_pixel_stride == 1) &&
               (src1_pixel_stride == 1) &&
               (src2_pixel_stride == 1) &&
               (srcA_pixel_stride == 1)) {
		unsigned int y_size = ci.ysize;

		do {
		    //
                    //  Point to the first pixel of the scanline
                    //
		    Xil_unsigned8* src1_pixel = src1_scanline;
		    Xil_unsigned8* src2_pixel = src2_scanline;
		    Xil_unsigned8* dest_pixel = dest_scanline;
		    Xil_unsigned8* alpha_pixel = srcA_scanline;
		    
		    Xil_unsigned32	ratio2;
		    BLEND_TYPE *tab1;
		    BLEND_TYPE *tab2;
		    int last_pixels;
		    
		    int pixel_count = ci.xsize;
		    int first_pixels = (((int)alpha_pixel) & 0x3);
		    if(first_pixels) {
			first_pixels = 4 - first_pixels;
                        
			if(first_pixels > pixel_count) {
			    first_pixels = pixel_count;
                        }

			pixel_count -= first_pixels;
			do {
			    ratio2 = (Xil_unsigned32) *alpha_pixel++;
			    tab2 = blend_table[ratio2];
			    tab1 = blend_table[XIL_MAXBYTE - ratio2];
			    
			    *dest_pixel++ =
				BLEND(tab1[*src1_pixel++],tab2[*src2_pixel++]);
			} while(--first_pixels);
		    }

		    //
                    //  For each pixel
                    //
		    last_pixels = pixel_count & 0x3;
		    pixel_count >>= 2;
		    ratio2 = 0;
		    tab2 = blend_table[0];
		    tab1 = blend_table[XIL_MAXBYTE];
		    while(pixel_count){
			pixel_count--;
			Xil_unsigned32 tmp;
			
			tmp = *((Xil_unsigned32*) alpha_pixel);
			alpha_pixel += 4;
			//
			// Maybe all 4 alpha values are the same?
			// Special case all 0 or all 1
			//
			if(ratio2 == tmp) {
			    if((tmp == 0) || (~tmp == 0)) {
                                //
				// alpha is all 0 or all 1, so just do copy
                                //
				size_t n_copy = 4;
				Xil_unsigned8 *src_pixels;
				if(tmp == 0)
				    src_pixels = src1_pixel;
				else
				    src_pixels = src2_pixel;
				//
				//  Count number of 4-pixels to copy
				//
				while(pixel_count>0) {
				    tmp = *((Xil_unsigned32*) alpha_pixel);
				    pixel_count--;
				    alpha_pixel += 4;
				    if (ratio2 != tmp)
					break;
				    n_copy += 4;
				}

				xili_memcpy(dest_pixel, src_pixels, n_copy);

				src1_pixel+=n_copy;
				src2_pixel+=n_copy;
				dest_pixel+=n_copy;

				//
				// If no more alphas, and we already processed
				// the last 4 alphas, then  just get out
				// of the 4-at-a-time loop
				//
				if((pixel_count<=0) && (ratio2 == tmp)) {
				    break;
                                }
			    } else {
				//
				// See if all 4 alpha values are equal
				//
				if(((tmp >> 16) == ((tmp<<16)>>16)) &&
                                   (tmp >> 24) == (tmp & 255)) {
				    do {
					dest_pixel[0] = 
					    BLEND(tab1[src1_pixel[0]],
						  tab2[src2_pixel[0]]);
					dest_pixel[1] =
					    BLEND(tab1[src1_pixel[1]],
						  tab2[src2_pixel[1]]);
					dest_pixel[2] = 
					    BLEND(tab1[src1_pixel[2]],
						  tab2[src2_pixel[2]]);
					dest_pixel[3] = 
					    BLEND(tab1[src1_pixel[3]],
						  tab2[src2_pixel[3]]);

					src1_pixel+=4;
					src2_pixel+=4;
					dest_pixel+=4;

					if(pixel_count > 0) {
					    tmp = *((Xil_unsigned32*)alpha_pixel);

					    pixel_count--;
					    alpha_pixel += 4;

					    if(ratio2 == tmp) {
						continue;
                                            }
					}
					break;
				    } while (1);
                                }

				if(!pixel_count && (ratio2 == tmp)) {
				    break;;
                                }
			    }
			}

			//
			//  Process 4 alpha values from the 4 saved in 'tmp'
			//
			ratio2 = GET_FIRST_BYTE(tmp);
			tab2 = blend_table[ratio2];
			tab1 = blend_table[XIL_MAXBYTE - ratio2];
			dest_pixel[0] = 
			    BLEND(tab1[src1_pixel[0]],tab2[src2_pixel[0]]);

			ratio2 = GET_SECOND_BYTE(tmp);
			tab2 = blend_table[ratio2];
			tab1 = blend_table[XIL_MAXBYTE - ratio2];
			dest_pixel[1] = 
			    BLEND(tab1[src1_pixel[1]],tab2[src2_pixel[1]]);

			ratio2 = GET_THIRD_BYTE(tmp);
			tab2 = blend_table[ratio2];
			tab1 = blend_table[XIL_MAXBYTE - ratio2];
			dest_pixel[2] =
			    BLEND(tab1[src1_pixel[2]],tab2[src2_pixel[2]]);

			ratio2 = GET_FOURTH_BYTE(tmp);
			tab2 = blend_table[ratio2];
			tab1 = blend_table[XIL_MAXBYTE - ratio2];
			dest_pixel[3] = 
			    BLEND(tab1[src1_pixel[3]],tab2[src2_pixel[3]]);
			ratio2 = tmp;

			src1_pixel+=4;
			src2_pixel+=4;
			dest_pixel+=4;
			
		    }
		    
		    if(last_pixels) {
			do {
			    ratio2 = (Xil_unsigned32) *alpha_pixel++;
			    tab2 = blend_table[ratio2];
			    tab1 = blend_table[XIL_MAXBYTE - ratio2];
			    
			    *dest_pixel++ =
				BLEND(tab1[*src1_pixel++],tab2[*src2_pixel++]);
			} while(--last_pixels);
		    }
		    
		    //
                    //  Move to the next scanline
                    //
                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
		} while(--y_size);
	    } else if((nbands            == 3) &&
                      (dest_pixel_stride == 3) &&
                      (src1_pixel_stride == 3) &&
                      (src2_pixel_stride == 3) &&
                      (srcA_pixel_stride == 1)) {
		int y_size = ci.ysize;
		do {
		    //
                    //  Point to the first pixel of the scanline
                    //
		    Xil_unsigned8* src1_pixel = src1_scanline;
		    Xil_unsigned8* src2_pixel = src2_scanline;
		    Xil_unsigned8* dest_pixel = dest_scanline;
		    Xil_unsigned8* alpha_pixel = srcA_scanline;
		    
		    Xil_unsigned32  ratio2;
		    BLEND_TYPE *tab1;
		    BLEND_TYPE *tab2;
		    int first_pixels;
		    int last_pixels;
		    
		    int pixel_count = ci.xsize;
		    first_pixels = (((int)alpha_pixel) & 0x3);
		    if(first_pixels) {
			first_pixels = 4 - first_pixels;
			if(first_pixels > pixel_count) {
			    first_pixels = pixel_count;
                        }
			pixel_count -= first_pixels;

			do {
			    ratio2 = (Xil_unsigned32) *alpha_pixel++;
			    tab2 = blend_table[ratio2];
			    tab1 = blend_table[XIL_MAXBYTE - ratio2];
			    
			    dest_pixel[0] =
                                BLEND(tab1[src1_pixel[0]],tab2[src2_pixel[0]]);
			    dest_pixel[1] =
                                BLEND(tab1[src1_pixel[1]],tab2[src2_pixel[1]]);
			    dest_pixel[2] =
                                BLEND(tab1[src1_pixel[2]],tab2[src2_pixel[2]]);
			    
			    //
                            //  Move to the next pixel
                            //
			    src1_pixel+=3;
			    src2_pixel+=3;
			    dest_pixel+=3;
			} while(--first_pixels);
		    }
		    
		    //
                    //  For each pixel
                    //
		    
		    last_pixels = pixel_count & 0x3;
		    pixel_count >>= 2;
		    ratio2 = 0;
		    tab2 = blend_table[0];
		    tab1 = blend_table[XIL_MAXBYTE];
		    while(pixel_count)
		    {
			pixel_count--;
			Xil_unsigned32 tmp;
			
			tmp = *((Xil_unsigned32*) alpha_pixel);
			alpha_pixel += 4;

			//
                        //  Maybe all 4 alpha values are the same?
			//    Special case all 0 or all 1
			//
			if(ratio2 == tmp) {
			    if((tmp == 0) || (~tmp == 0)) {
                                //
				//  Alpha is all 0 or all 1, so just do copy
                                //
				size_t n_copy = 12;
				Xil_unsigned8 *src_pixels;
				if(tmp == 0) {
				    src_pixels = src1_pixel;
				} else {
				    src_pixels = src2_pixel;
                                }

				//
				//  Count number of additional 4-pixels to copy
				//
				while(pixel_count > 0) {
				    tmp = *((Xil_unsigned32*) alpha_pixel);
				    alpha_pixel += 4;
				    pixel_count--;

				    if(ratio2 != tmp) {
					break;
                                    }

				    n_copy += 12;
				}

				xili_memcpy(dest_pixel, src_pixels, n_copy);

				src1_pixel+=n_copy;
				src2_pixel+=n_copy;
				dest_pixel+=n_copy;

				//
				//  If no more alphas, and we already processed
				//  the last 4 alphas, then  just get out
				//  of the 4-at-a-time loop
				//  otherwise, tmp contains the last 4 alphas
				//  read, and alpha_pixel is positioned
				//  correctly.
				//
				if((pixel_count<=0) && (ratio2 == tmp)) {
				    break;
                                }
			    } else {
				//
				//  See if all 4 alpha values are equal
				//
				if(((tmp >> 16) == ((tmp<<16)>>16)) &&
                                   (tmp >> 24) == (tmp & 255)) {
				    do {
					dest_pixel[0] =
					    BLEND(tab1[src1_pixel[0]],
						  tab2[src2_pixel[0]]);
					dest_pixel[1] =
					    BLEND(tab1[src1_pixel[1]],
						  tab2[src2_pixel[1]]);
					dest_pixel[2] =
					    BLEND(tab1[src1_pixel[2]],
						  tab2[src2_pixel[2]]);
					dest_pixel[3] =
					    BLEND(tab1[src1_pixel[3]],
						  tab2[src2_pixel[3]]);
					dest_pixel[4] =
					    BLEND(tab1[src1_pixel[4]],
						  tab2[src2_pixel[4]]);
					dest_pixel[5] =
					    BLEND(tab1[src1_pixel[5]],
						  tab2[src2_pixel[5]]);
					dest_pixel[6] =
					    BLEND(tab1[src1_pixel[6]],
						  tab2[src2_pixel[6]]);
					dest_pixel[7] =
					    BLEND(tab1[src1_pixel[7]],
						  tab2[src2_pixel[7]]);
					dest_pixel[8] =
					    BLEND(tab1[src1_pixel[8]],
						  tab2[src2_pixel[8]]);
					dest_pixel[9] =
					    BLEND(tab1[src1_pixel[9]],
						  tab2[src2_pixel[9]]);
					dest_pixel[10] =
					    BLEND(tab1[src1_pixel[10]],
						  tab2[src2_pixel[10]]);
					dest_pixel[11] =
					    BLEND(tab1[src1_pixel[11]],
						  tab2[src2_pixel[11]]);

					src1_pixel+=12;
					src2_pixel+=12;
					dest_pixel+=12;

					if(pixel_count>0) {
					    tmp = *((Xil_unsigned32*) alpha_pixel);
					    pixel_count--;
					    alpha_pixel += 4;

					    if(ratio2 == tmp) {
						continue;
                                            }
					}
					break;
				    } while(1);
                                }

                                if(!pixel_count && (ratio2 == tmp)) {
                                    break;
                                }
			    }
			}

			//
			//  Process 4 alpha values from the 4 saved  in 'tmp'
			//
			ratio2 = GET_FIRST_BYTE(tmp);
			tab2 = blend_table[ratio2];
			tab1 = blend_table[XIL_MAXBYTE - ratio2];
			dest_pixel[0] =
			    BLEND(tab1[src1_pixel[0]],tab2[src2_pixel[0]]);
			dest_pixel[1] =
			    BLEND(tab1[src1_pixel[1]],tab2[src2_pixel[1]]);
			dest_pixel[2] =
			    BLEND(tab1[src1_pixel[2]],tab2[src2_pixel[2]]);

			ratio2 = GET_SECOND_BYTE(tmp);
			tab2 = blend_table[ratio2];
			tab1 = blend_table[XIL_MAXBYTE - ratio2];
			dest_pixel[3] =
			    BLEND(tab1[src1_pixel[3]],tab2[src2_pixel[3]]);
			dest_pixel[4] =
			    BLEND(tab1[src1_pixel[4]],tab2[src2_pixel[4]]);
			dest_pixel[5] =
			    BLEND(tab1[src1_pixel[5]],tab2[src2_pixel[5]]);

			ratio2 = GET_THIRD_BYTE(tmp);
			tab2 = blend_table[ratio2];
			tab1 = blend_table[XIL_MAXBYTE - ratio2];
			dest_pixel[6] =
			    BLEND(tab1[src1_pixel[6]],tab2[src2_pixel[6]]);
			dest_pixel[7] =
			    BLEND(tab1[src1_pixel[7]],tab2[src2_pixel[7]]);
			dest_pixel[8] =
			    BLEND(tab1[src1_pixel[8]],tab2[src2_pixel[8]]);

			ratio2 = GET_FOURTH_BYTE(tmp);
			tab2 = blend_table[ratio2];
			tab1 = blend_table[XIL_MAXBYTE - ratio2];
			dest_pixel[9] =
			    BLEND(tab1[src1_pixel[9]],tab2[src2_pixel[9]]);
			dest_pixel[10] =
			    BLEND(tab1[src1_pixel[10]],tab2[src2_pixel[10]]);
			dest_pixel[11] =
			    BLEND(tab1[src1_pixel[11]],tab2[src2_pixel[11]]);
			ratio2 = tmp;

			src1_pixel+=12;
			src2_pixel+=12;
			dest_pixel+=12;
			
		    }
		    
		    if(last_pixels) {
			do {
			    ratio2 = (Xil_unsigned32) *alpha_pixel++;
			    tab2 = blend_table[ratio2];
			    tab1 = blend_table[XIL_MAXBYTE - ratio2];
			    
			    dest_pixel[0] =
                                BLEND(tab1[src1_pixel[0]],tab2[src2_pixel[0]]);
			    dest_pixel[1] =
                                BLEND(tab1[src1_pixel[1]],tab2[src2_pixel[1]]);
			    dest_pixel[2] =
                                BLEND(tab1[src1_pixel[2]],tab2[src2_pixel[2]]);
			    
			    //
                            //  Move to the next pixel
                            //
			    src1_pixel+=3;
			    src2_pixel+=3;
			    dest_pixel+=3;
			} while(--last_pixels);
		    }
		    
		    //
                    //  Move to the next scanline
                    //
                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
		} while(--y_size);
	    } else {
		//
		// General pixel sequential case
		//
                for(y=ci.ysize; y>0; y--) {
                    Xil_unsigned8* src1_pixel = src1_scanline;
                    Xil_unsigned8* src2_pixel = src2_scanline;
                    Xil_unsigned8* srcA_pixel = srcA_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        Xil_unsigned8* src1 = src1_pixel;
                        Xil_unsigned8* src2 = src2_pixel;
                        Xil_unsigned8* srcA = srcA_pixel;
                        Xil_unsigned8* dest = dest_pixel;

                        Xil_unsigned32  ratio = *srcA;
                        BLEND_TYPE* tab1;
                        BLEND_TYPE* tab2;

                        for(band=0; band<ci.destNumBands; band++) {
                            tab2  = blend_table[ratio];
                            tab1  = blend_table[XIL_MAXBYTE - ratio];

                            *dest = BLEND(tab1[*src1], tab2[*src2]);

                            src1++;
                            src2++;
                            dest++;
                        }

                        src1_pixel += src1_pixel_stride;
                        src2_pixel += src2_pixel_stride;
                        srcA_pixel += srcA_pixel_stride;
                        dest_pixel += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	} else {
            //
            //  The Alpha image.
            //
            Xil_unsigned8* srcA_data            = (Xil_unsigned8*) ci.src3Data;
            unsigned int   srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int   srcA_pixel_stride    = ci.src3PixelStride;

	    for(band=0; band<nbands; band++) { 
                //
                //  The input images.
                //
                Xil_unsigned8* src1_scanline        = (Xil_unsigned8*)
                    ci.getSrc1Data(band);
                Xil_unsigned8* src2_scanline        = (Xil_unsigned8*)
                    ci.getSrc2Data(band);
                Xil_unsigned8* dest_scanline        = (Xil_unsigned8*)
                    ci.getDestData(band);

                unsigned int   src1_scanline_stride =
                    ci.getSrc1ScanlineStride(band);
                unsigned int   src2_scanline_stride =
                    ci.getSrc2ScanlineStride(band);
                unsigned int   dest_scanline_stride =
                    ci.getDestScanlineStride(band);

                unsigned int   src1_pixel_stride    =
                    ci.getSrc1PixelStride(band);
                unsigned int   src2_pixel_stride    =
                    ci.getSrc2PixelStride(band);
                unsigned int   dest_pixel_stride    =
                    ci.getDestPixelStride(band);

                Xil_unsigned8* srcA_scanline        = srcA_data;

                for(y=ci.ysize; y>0; y--) { 
                    Xil_unsigned8* src1_pixel = src1_scanline;
                    Xil_unsigned8* src2_pixel = src2_scanline;
                    Xil_unsigned8* srcA_pixel = srcA_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        //
                        //  General case - (arbitrary or band sequential)
                        //
                        Xil_unsigned32  ratio = *srcA_pixel;

                        BLEND_TYPE*     tab2  = blend_table[ratio];
                        BLEND_TYPE*     tab1  = blend_table[XIL_MAXBYTE - ratio];
		    
                        *dest_pixel = BLEND(tab1[*src1_pixel], tab2[*src2_pixel]);
		    
                        src1_pixel += src1_pixel_stride;
                        src2_pixel += src2_pixel_stride;
                        srcA_pixel += srcA_pixel_stride;
                        dest_pixel += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	}
    }

    return ci.returnValue;
}


XilStatus
XilDeviceManagerComputeBYTE::Blenda16(XilOp*       op,
				      unsigned     op_count,
				      XilRoi*      roi,
				      XilBoxList*  bl)

{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands      = ci.destNumBands;
    float        short_range = (float)XIL_MAXSHORT - (float)XIL_MINSHORT;

    //
    //  Loop iterators.
    //
    int          x;
    int          y;

    while(ci.hasMoreInfo()) {
	if((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.src2Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)))
	{
            //
            //  The Alpha image.
            //
            Xil_signed16* srcA_scanline        = (Xil_signed16*)ci.src3Data;
            unsigned int  srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int  srcA_pixel_stride    = ci.src3PixelStride;

            //
            //  The input images.
            //
	    Xil_unsigned8* src1_scanline        = (Xil_unsigned8*)ci.src1Data;
            Xil_unsigned8* src2_scanline        = (Xil_unsigned8*)ci.src2Data;
            Xil_unsigned8* dest_scanline        = (Xil_unsigned8*)ci.destData;

            unsigned int  src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int  src2_scanline_stride = ci.src2ScanlineStride;
            unsigned int  dest_scanline_stride = ci.destScanlineStride;

            unsigned int  src1_pixel_stride    = ci.src1PixelStride;
            unsigned int  src2_pixel_stride    = ci.src2PixelStride;
            unsigned int  dest_pixel_stride    = ci.destPixelStride;

            if(nbands == 1) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_unsigned8* src1 = src1_scanline;
                    Xil_unsigned8* src2 = src2_scanline;
                    Xil_unsigned8* dest = dest_scanline;
                    Xil_signed16*  srcA = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float ratio =
                            ((float)(*srcA) - (float)XIL_MINSHORT) / short_range;
                        float sum   =
                            ((1.0F - ratio) * *src1) +
                            (ratio          * *src2);

                        *dest = (Xil_unsigned8)(sum + 0.5F);

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else if(nbands == 3) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_unsigned8* src1 = src1_scanline;
                    Xil_unsigned8* src2 = src2_scanline;
                    Xil_unsigned8* dest = dest_scanline;
                    Xil_signed16*  srcA = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float ratio =
                            ((float)(*srcA) - (float)XIL_MINSHORT) / short_range;

                        float sum   =
                            ((1.0F - ratio) * *src1) +
                            (ratio          * *src2);
                        *dest = (Xil_unsigned8)(sum + 0.5F);

                        sum =
                            ((1.0F - ratio) * *(src1+1)) +
                            (ratio          * *(src2+1));
                        *(dest+1) = (Xil_unsigned8)(sum + 0.5F);

                        sum =
                            ((1.0F - ratio) * *(src1+2)) +
                            (ratio          * *(src2+2));
                        *(dest+2) = (Xil_unsigned8)(sum + 0.5F);

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else {
                for(y=ci.ysize; y>0; y--) {
                    Xil_unsigned8* src1_pixel = src1_scanline;
                    Xil_unsigned8* src2_pixel = src2_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;
                    Xil_signed16*  srcA_pixel = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        Xil_unsigned8* src1 = src1_pixel;
                        Xil_unsigned8* src2 = src2_pixel;
                        Xil_unsigned8* dest = dest_pixel;
                        Xil_signed16*  srcA = srcA_pixel;

                        float ratio =
                            ((float)(*srcA) - (float)XIL_MINSHORT) / short_range;

                        for(unsigned int band=0; band<nbands; band++) {
                            float sum =
                                ((1.0F - ratio) * *src1) +
                                (ratio          * *src2);

                            *dest = (Xil_unsigned8)(sum + 0.5F);

                            src1++;
                            src2++;
                            dest++;
                        }

                        src1_pixel += src1_pixel_stride;
                        src2_pixel += src2_pixel_stride;
                        srcA_pixel += srcA_pixel_stride;
                        dest_pixel += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	} else {
            //
            //  The Alpha image.
            //
            Xil_signed16* srcA_data            = (Xil_signed16*)ci.src3Data;
            unsigned int  srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int  srcA_pixel_stride    = ci.src3PixelStride;

	    for(unsigned int band=0; band<nbands; band++) { 
                //
                //  The input images.
                //
                Xil_unsigned8*  src1_scanline        = (Xil_unsigned8*)
                    ci.getSrc1Data(band);
                Xil_unsigned8*  src2_scanline        = (Xil_unsigned8*)
                    ci.getSrc2Data(band);
                Xil_unsigned8*  dest_scanline        = (Xil_unsigned8*)
                    ci.getDestData(band);

                unsigned int  src1_scanline_stride =
                    ci.getSrc1ScanlineStride(band);
                unsigned int  src2_scanline_stride =
                    ci.getSrc2ScanlineStride(band);
                unsigned int  dest_scanline_stride =
                    ci.getDestScanlineStride(band);

                unsigned int  src1_pixel_stride    =
                    ci.getSrc1PixelStride(band);
                unsigned int  src2_pixel_stride    =
                    ci.getSrc2PixelStride(band);
                unsigned int  dest_pixel_stride    =
                    ci.getDestPixelStride(band);

                Xil_signed16* srcA_scanline        = srcA_data;

                for(y=ci.ysize; y>0; y--) { 
                    Xil_unsigned8* src1 = src1_scanline;
                    Xil_unsigned8* src2 = src2_scanline;
                    Xil_unsigned8* dest = dest_scanline;
                    Xil_signed16* srcA = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float ratio =
                            ((float)(*srcA) - (float)XIL_MINSHORT) / short_range;
                        float sum   =
                            ((1.0F - ratio) * *src1) +
                            (ratio          * *src2);

                        *dest = (Xil_unsigned8)(sum + 0.5F);


                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	}
    }
    
    return ci.returnValue;
}



XilStatus
XilDeviceManagerComputeBYTE::Blendaf32(XilOp*       op,
				       unsigned     op_count,
				       XilRoi*      roi,
				       XilBoxList*  bl)

{
    ComputeInfoGENERAL  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return ci.returnValue;
    }

    unsigned int nbands = ci.destNumBands;

    //
    //  Loop iterators.
    //
    int          x;
    int          y;

    while(ci.hasMoreInfo()) {
	if((ci.src1Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.src2Storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (ci.destStorage.isType(XIL_PIXEL_SEQUENTIAL)))
	{
            //
            //  The Alpha image.
            //
            Xil_float32*  srcA_data            = (Xil_float32*) ci.src3Data;
            unsigned int  srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int  srcA_pixel_stride    = ci.src3PixelStride;

            //
            //  The input images.
            //
	    Xil_unsigned8* src1_scanline        = (Xil_unsigned8*) ci.src1Data;
            Xil_unsigned8* src2_scanline        = (Xil_unsigned8*) ci.src2Data;
            Xil_unsigned8* dest_scanline        = (Xil_unsigned8*) ci.destData;

            unsigned int  src1_scanline_stride = ci.src1ScanlineStride;
            unsigned int  src2_scanline_stride = ci.src2ScanlineStride;
            unsigned int  dest_scanline_stride = ci.destScanlineStride;

            unsigned int  src1_pixel_stride    = ci.src1PixelStride;
            unsigned int  src2_pixel_stride    = ci.src2PixelStride;
            unsigned int  dest_pixel_stride    = ci.destPixelStride;

            Xil_float32*  srcA_scanline        = srcA_data;

            if(nbands == 1) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_unsigned8* src1 = src1_scanline;
                    Xil_unsigned8* src2 = src2_scanline;
                    Xil_unsigned8* dest = dest_scanline;
                    Xil_float32*  srcA = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float sum   =
                            ((1.0F - *srcA) * *src1) +
                            (*srcA          * *src2);

                        *dest = (Xil_unsigned8)(sum + 0.5F);

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else if(nbands == 3) {
                for(y=ci.ysize; y>0; y--) {
                    Xil_unsigned8* src1 = src1_scanline;
                    Xil_unsigned8* src2 = src2_scanline;
                    Xil_unsigned8* dest = dest_scanline;
                    Xil_float32*  srcA = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float sum   =
                            ((1.0F - *srcA) * *src1) +
                            (*srcA          * *src2);
                        *dest = (Xil_unsigned8)(sum + 0.5F);

                        sum =
                            ((1.0F - *srcA) * *(src1+1)) +
                            (*srcA          * *(src2+1));
                        *(dest+1) = (Xil_unsigned8)(sum + 0.5F);

                        sum =
                            ((1.0F - *srcA) * *(src1+2)) +
                            (*srcA          * *(src2+2));
                        *(dest+2) = (Xil_unsigned8)(sum + 0.5F);

                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
            } else {
                for(y=ci.ysize; y>0; y--) {
                    Xil_unsigned8* src1_pixel = src1_scanline;
                    Xil_unsigned8* src2_pixel = src2_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;
                    Xil_float32*  srcA_pixel = srcA_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        Xil_unsigned8*  src1 = src1_pixel;
                        Xil_unsigned8*  src2 = src2_pixel;
                        Xil_float32*    srcA = srcA_pixel;
                        Xil_unsigned8*  dest = dest_pixel;

                        for(unsigned int band=0; band<nbands; band++) {
                            float sum =
                                ((1.0F - *srcA) * *src1) +
                                (*srcA          * *src2);

                            *dest = (Xil_unsigned8)(sum + 0.5F);

                            src1++;
                            src2++;
                            dest++;
                        }

                        src1_pixel += src1_pixel_stride;
                        src2_pixel += src2_pixel_stride;
                        srcA_pixel += srcA_pixel_stride;
                        dest_pixel += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	} else {
            //
            //  The Alpha image.
            //
            Xil_float32* srcA_data            = (Xil_float32*) ci.src3Data;
            unsigned int srcA_scanline_stride = ci.src3ScanlineStride;
            unsigned int srcA_pixel_stride    = ci.src3PixelStride;

	    for(unsigned int band=0; band<nbands; band++) { 
                //
                //  The input images.
                //
                Xil_unsigned8* src1_scanline        = (Xil_unsigned8*)
                    ci.getSrc1Data(band);
                Xil_unsigned8* src2_scanline        = (Xil_unsigned8*)
                    ci.getSrc2Data(band);
                Xil_unsigned8* dest_scanline        = (Xil_unsigned8*)
                    ci.getDestData(band);

                unsigned int  src1_scanline_stride =
                    ci.getSrc1ScanlineStride(band);
                unsigned int  src2_scanline_stride =
                    ci.getSrc2ScanlineStride(band);
                unsigned int  dest_scanline_stride =
                    ci.getDestScanlineStride(band);

                unsigned int  src1_pixel_stride    =
                    ci.getSrc1PixelStride(band);
                unsigned int  src2_pixel_stride    =
                    ci.getSrc2PixelStride(band);
                unsigned int  dest_pixel_stride    =
                    ci.getDestPixelStride(band);

                Xil_float32* srcA_scanline         = srcA_data;

                for(y=ci.ysize; y>0; y--) { 
                    Xil_unsigned8* src1 = src1_scanline;
                    Xil_unsigned8* src2 = src2_scanline;
                    Xil_float32*   srcA = srcA_scanline;
                    Xil_unsigned8* dest = dest_scanline;

                    for(x=ci.xsize; x>0; x--) {
                        float sum   =
                            ((1.0F - *srcA) * *src1) +
                            (*srcA          * *src2);

                        *dest = (Xil_unsigned8)(sum + 0.5F);


                        src1 += src1_pixel_stride;
                        src2 += src2_pixel_stride;
                        srcA += srcA_pixel_stride;
                        dest += dest_pixel_stride;
                    }

                    src1_scanline += src1_scanline_stride;
                    src2_scanline += src2_scanline_stride;
                    srcA_scanline += srcA_scanline_stride;
                    dest_scanline += dest_scanline_stride;
                }
	    }
	}
    }
    
    return ci.returnValue;
}


