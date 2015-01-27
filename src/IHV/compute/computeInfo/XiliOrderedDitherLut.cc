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
//  File:       XiliOrderedDitherLut.cc
//  Project:    XIL
//  Revision:   1.15
//  Last Mod:   10:13:36, 03/10/00
//
//  Description:
//
//    Implementation of Ordered Dither for BYTE sources.
//    Special cases are provided for 3 band - 32 bit aligned
//    and <= 4 band, Dither Size <= 16x16
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XiliOrderedDitherLut.cc	1.15\t00/03/10  "

#include "XiliOrderedDitherLut.hh"
#include "XiliUtils.hh"

//
// Macro to clamp an int to an unsigned byte range
// The argument must be a simple variable, for this macro 
// to function properly.
//
#define CLAMP(k) {       \
  if(k & 0xffffff00) {   \
      if(k > 255) {      \
          k = 255;       \
      } else { \
          k = 0;         \
      }                  \
  }                      \
}

#define NGRAY 512

XiliOrderedDitherLut::XiliOrderedDitherLut(XilLookupColorcube*     cmap,
                                           XilDitherMask*          dmask)
{
    isOKFlag = FALSE;

    XilSystemState* err_state = cmap->getSystemState();

    nbands = cmap->getOutputNBands();

    //
    // Get dither mask parameters
    // Scale dither threshold values to 0-255 range
    //
    dmat_w     = dmask->getWidth();
    dmat_h     = dmask->getHeight();
    dmat_sz    = dmat_w * dmat_h;

    unsigned int mult_table_size = nbands * sizeof(int);
    unsigned int dims_table_size = nbands * sizeof(unsigned int);
    unsigned int dmat_table_size = (nbands*dmat_sz + 3) & ~3;

    unsigned int total_table_size;
    total_table_size = mult_table_size
                     + dims_table_size
                     + dmat_table_size;

    //
    // If the size if small enough, allocate a table (dlut)
    // to implement the dither operation.
    //
    dlut_band_stride  = dmat_sz * 256;
    dlut_row_stride  = dmat_w * 256;
    dlut_col_stride  = 256;

    unsigned int dlut_table_size = (nbands * dlut_band_stride + 3) & ~3;
    if(dlut_table_size <= _XIL_OD_MAX_LUT_SIZE) {
        total_table_size += dlut_table_size;
    }


    multi_table = NULL;
    mults       = NULL;
    dims        = NULL;
    dmat        = NULL;
    dlut        = NULL;

    //
    // Set the special case enum to the default - OD_GENERAL
    // This will be overridden if special cases are possible
    //
    special_case = OD_GENERAL;

    //
    // Allocate a single table which can hold all
    // of the individual tables.
    //
    multi_table = (Xil_unsigned8*) (new int[total_table_size/4]);
    if(multi_table == NULL) {
        XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    //
    // Set pointers to each of the tables with the big array
    //
    mults = (int*) multi_table;
    dims = (unsigned int*) (multi_table + mult_table_size);
    dmat = multi_table + (mult_table_size + dims_table_size);
    dlut = dmat + dmat_table_size;

    xili_memcpy(mults, cmap->getMultipliers(), nbands*sizeof(int));
    xili_memcpy(dims,  cmap->getDimsMinus1(), nbands*sizeof(unsigned int));

    cmap_offset = cmap->getAdjustedOffset();

    const float* mat_values = dmask->getData();

    //
    // Scale dither threshold values to 0-255 range
    //
    for(unsigned int i=0; i<nbands*dmat_sz; i++) {
        dmat[i] = (Xil_unsigned8)(mat_values[i] * 255.0F);
    }

    if(dlut_table_size <= _XIL_OD_MAX_LUT_SIZE) {
        //
        // Set to OD_3_BAND or OD_N_BAND special case, We can't tell about
        // the alignment inside this routine, since it can
        // change on each rect.
        //
        if(nbands == 3) {
            special_case = OD_3_BAND;
        } else {
            special_case = OD_N_BAND;
        }
    } else {
        //
        // The table would be too large. 
        // We must use the OD_GENERAL code.
        //
        special_case = OD_GENERAL;
        return;
    }

    //
    // Construct the big dither table. If indexed as a
    // multi-dimensional array this would be equivalent to:
    //
    //   ditherTab[band][ditherRow][ditherColumn][grayLevel]
    //
    // where ditherRow, Col are modulo the dither mask size.
    //
    // To minimize the table construction cost, precalculate
    // the bin value for a given band and gray level. Then use
    // the dithermask threshold value to determine whether to bump
    // the value up one level. Thus most of the work is done in
    // the outerloops, with a simple comparison left for the inner loop.
    //
    for(unsigned int band=0; band<nbands; band++) {
        int step  = dims[band];
        int delta = mults[band];
        Xil_unsigned8* pDmatBand = dmat + band*dmat_sz;
        Xil_unsigned8* pDithBand = dlut + band*dlut_band_stride;
        int sum = 0;
        for(int gray=0; gray<256; gray++) {
            int tmp = sum;
            int frac = (int)(tmp & 0xFF);
            int bin = tmp >> 8;
            Xil_unsigned8 lowVal = bin * delta;
            Xil_unsigned8 highVal = lowVal + delta;
            Xil_unsigned8* pDmat = pDmatBand;
            Xil_unsigned8* pDith = pDithBand + gray;
            for(unsigned int dcount=0; dcount<dmat_sz; dcount++) {
                int threshold = *pDmat++;
                if(frac > threshold) {
                    *pDith = highVal;
                } else {
                    *pDith = lowVal;
                }
                pDith += 256;
            } // end dithermask entry
            sum += step;
        } // end gray level
    } // end band

    isOKFlag = TRUE;
    return;
}


//
// Special constructor for YUV411 dither molecules
//
XiliOrderedDitherLut::XiliOrderedDitherLut(XilLookupColorcube*     cmap,
                                           XilDitherMask*          dmask,
                                           float*                  scale_arg,
                                           float*                  off_arg,
                                           int                     tbl_offset)
{
    float scale[3] = {1.0F, 1.0F, 1.0F};
    float off[3]   = {0.0F, 0.0F, 0.0F};

    isOKFlag = FALSE;

    XilSystemState* err_state = cmap->getSystemState();

    Xil_boolean do_rescale = FALSE;
    if(scale_arg != NULL) {
        do_rescale = TRUE;
        scale[0] = scale_arg[0]; 
        scale[1] = scale_arg[1]; 
        scale[2] = scale_arg[2]; 
    }

    if(off_arg != NULL) {
        do_rescale = TRUE;
        off[0] = off_arg[0]; 
        off[1] = off_arg[1]; 
        off[2] = off_arg[2]; 
    }

    nbands = 3;

    //
    // Get dither mask parameters
    //
    dmat_w     = 4;
    dmat_h     = 4;
    dmat_sz    = 16;

    unsigned int mult_table_size = nbands * sizeof(int);
    unsigned int dims_table_size = nbands * sizeof(unsigned int);
    unsigned int dmat_table_size = nbands * 4 * 4;

    unsigned int total_table_size;
    total_table_size = mult_table_size
                     + dims_table_size
                     + dmat_table_size;

    unsigned int dlut_table_size = 4 * 4 * NGRAY                 // Y
                                 + 2 * 2 * NGRAY * sizeof(int)   // U
                                 + 2 * 2 * NGRAY * sizeof(int);  // V

    total_table_size += dlut_table_size;

    dlut_band_stride  = dmat_sz * NGRAY;
    dlut_row_stride  = dmat_w * NGRAY;
    dlut_col_stride  = NGRAY;


    multi_table = NULL;
    mults       = NULL;
    dims        = NULL;
    dmat        = NULL;
    dlut        = NULL;

    //
    // Allocate a single table which can hold all
    // of the individual tables.
    //
    multi_table = (Xil_unsigned8*) (new int[total_table_size/4]);
    if(multi_table == NULL) {
        XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    //
    // Set pointers to each of the tables with the big array
    //
    mults = (int*) multi_table;
    dims = (unsigned int*) (multi_table + mult_table_size);
    dmat = multi_table + (mult_table_size + dims_table_size);
    dlut = dmat + dmat_table_size;

    yDitherLut = dlut;
    uvDitherLut = (unsigned int*)(yDitherLut + 4*4*NGRAY);

    xili_memcpy(mults, cmap->getMultipliers(), nbands*sizeof(int));
    xili_memcpy(dims,  cmap->getDimsMinus1(), nbands*sizeof(unsigned int));

    cmap_offset = cmap->getAdjustedOffset();

    const float* mat_values = dmask->getData();

    //
    // Scale dither threshold values to 0-255 range
    //
    for(unsigned int i=0; i<nbands*dmat_sz; i++) {
        dmat[i] = (Xil_unsigned8)(mat_values[i] * 255.0F);
    }

    //
    // Construct the Y dither table
    // For this and the UV tables, create a guard band of
    // 128 gray levels abobe and below the valid range so that
    // we don't have to clamp.
    // TODO: Verify that this is sufficient, i.e that it covers
    //       the theoretical range of possible values.
    //
    int step  = dims[0];
    int delta = mults[0];
    int slot = 0;
    for(int igray=-128; igray<384; igray++) {
        int gray = (igray < 0) ? 0 : ((igray>255) ? 255 : igray);
        int g = gray;
        if(do_rescale) {
            //
            // Determine the rescaled gray level.
            // This is what will be used for calculating the
            // dither table entry for the unrescaled gray level
            //
            g = (int) (gray*scale[0] + off[0] + 0.5);
            CLAMP(g);
        }
        int tmp = g*step;
        int frac = (int)(tmp & 0xFF);
        int bin = tmp >> 8;
        Xil_unsigned8 lowVal = bin * delta;
        Xil_unsigned8 highVal = lowVal + delta;
        Xil_unsigned8* pDmat = dmat;
        Xil_unsigned8* pDith = yDitherLut + slot;
        for(unsigned int dcount=0; dcount<dmat_sz; dcount++) {
            int threshold = *pDmat++;
            if(frac > threshold) {
                *pDith = highVal;
            } else {
                *pDith = lowVal;
            }
            pDith += NGRAY;
        } // end dithermask entry
        slot++;
    } // end gray level

    //
    // Construct the U and V dither tables.
    // These are very specific to 4:1:1 video
    // Each entry has the dither index contributions
    // for all 4 values of a 2x2 block packed into an integer.
    // There will be four or these ints per gray level,
    // corresponding to the four possible positions within the
    // 4x4 dither matrix.
    //
    for(int band=1; band<=2; band++) {
        step  = dims[band];
        delta = mults[band];
        Xil_unsigned8* pDmatBand = dmat + band*dmat_sz;
        unsigned int*  pDithBand = uvDitherLut + (band-1) * 2 * 2 * NGRAY;
        slot = 0;
        for(igray=-128; igray<384; igray++) {
            int gray = (igray < 0) ? 0 : ((igray>255) ? 255 : igray);
            int g = gray;
            if(do_rescale) {
                g = (int) (gray*scale[band] + off[band] + 0.5);
                CLAMP(g);
            }
            int tmp = g*step;
            int frac = (int)(tmp & 0xFF);
            int bin = tmp >> 8;
            Xil_unsigned8 lowVal = bin * delta;
            Xil_unsigned8 highVal = lowVal + delta;
            unsigned int*  pDith = pDithBand + slot;

            //
            // Rows of 2x2 blocks
            //
            for(int brow=0; brow<2; brow++) {
                //
                // Columns of 2x2 blocks
                //
                for(int bcol=0; bcol<2; bcol++) {
                    int index_base = brow*8 + bcol*2;
                    unsigned int lutval = 0;
                    //
                    // Rows and columns within a 2x2 block
                    //
                    for(int row=0; row<2; row++) {
                        for(int col=0; col<2; col++) {
                            int index = index_base + row*4 + col;
                            int threshold = pDmatBand[index];
                            if(frac > threshold) {
                                lutval = (lutval<<8) | highVal;
                            } else {
                                lutval = (lutval<<8) | lowVal;
                            }
                        }
                    }
                    *pDith = lutval;
                    pDith += NGRAY;
                }
            }
            slot++;
        } // end gray level
    }

    //
    // Adjust the pointers to the dither tables to account for
    // any bias in the source data. This is necessary to deal
    // with the bias of 128 in Jpeg commpressed data. So we
    // just adjust the table addressing rather than the data.
    //
    yDitherLut  += tbl_offset + 128;
    uvDitherLut += tbl_offset + 128;


    isOKFlag = TRUE;
    return;
}

//
// Special constructor for CellB YUV dither molecules
//
XiliOrderedDitherLut::XiliOrderedDitherLut(XilLookupColorcube*     cmap,
                                           XilDitherMask*          dmask,
                                           float*                  scale_arg,
                                           float*                  off_arg)
{
    float scale[3] = {1.0F, 1.0F, 1.0F};
    float off[3]   = {0.0F, 0.0F, 0.0F};

    isOKFlag = FALSE;

    XilSystemState* err_state = cmap->getSystemState();

    Xil_boolean do_rescale = FALSE;
    if(scale_arg != NULL) {
        do_rescale = TRUE;
        scale[0] = scale_arg[0]; 
        scale[1] = scale_arg[1]; 
        scale[2] = scale_arg[2]; 
    }

    if(off_arg != NULL) {
        do_rescale = TRUE;
        off[0] = off_arg[0]; 
        off[1] = off_arg[1]; 
        off[2] = off_arg[2]; 
    }

    nbands = 3;

    //
    // Get dither mask parameters
    //
    dmat_w     = 4;
    dmat_h     = 4;
    dmat_sz    = 16;

    unsigned int mult_table_size = nbands * sizeof(int);
    unsigned int dims_table_size = nbands * sizeof(unsigned int);
    unsigned int dmat_table_size = nbands * 4 * 4;

    unsigned int total_table_size;
    total_table_size = mult_table_size
                     + dims_table_size
                     + dmat_table_size;

    unsigned int dlut_table_size = 3 * 4 * 4 * 256;

    total_table_size += dlut_table_size;

    dlut_band_stride  = dmat_sz * 256;
    dlut_row_stride  = dmat_w * 256;
    dlut_col_stride  = 256;

    multi_table = NULL;
    mults       = NULL;
    dims        = NULL;
    dmat        = NULL;
    dlut        = NULL;

    //
    // Allocate a single table which can hold all
    // of the individual tables.
    //
    multi_table = (Xil_unsigned8*) (new int[total_table_size/4]);
    if(multi_table == NULL) {
        XIL_ERROR(err_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    //
    // Set pointers to each of the tables with the big array
    //
    mults = (int*) multi_table;
    dims = (unsigned int*) (multi_table + mult_table_size);
    dmat = multi_table + (mult_table_size + dims_table_size);
    dlut = dmat + dmat_table_size;

    xili_memcpy(mults, cmap->getMultipliers(), nbands*sizeof(int));
    xili_memcpy(dims,  cmap->getDimsMinus1(), nbands*sizeof(unsigned int));

    cmap_offset = cmap->getAdjustedOffset();

    cmap_base32 = (cmap_offset<<8) | cmap_offset;
    cmap_base32 = (cmap_base32<<16) | cmap_base32;

    const float* mat_values = dmask->getData();

    //
    // Modify the 
    //
    // Scale dither threshold values to 0-255 range
    //
    for(unsigned int i=0; i<nbands*dmat_sz; i++) {
        dmat[i] = (Xil_unsigned8)(mat_values[i] * 255.0F);
    }

    //
    // Construct the big dither table. If indexed as a
    // multi-dimensional array this would be equivalent to:
    //
    //   ditherTab[band][ditherRow][ditherColumn][grayLevel]
    //
    // where ditherRow, Col are modulo the dither mask size.
    //
    // To minimize the table construction cost, precalculate
    // the bin value for a given band and gray level. Then use
    // the dithermask threshold value to determine whether to bump
    // the value up one level. Thus most of the work is done in
    // the outerloops, with a simple comparison left for the inner loop.
    //
    for(unsigned int band=0; band<nbands; band++) {
        int step  = dims[band];
        int delta = mults[band];
        Xil_unsigned8* pDmatBand = dmat + band*dmat_sz;
        Xil_unsigned8* pDithBand = dlut + band*dlut_band_stride;
        for(int gray=0; gray<256; gray++) {
            int g = gray;
            if(do_rescale) {
                g = (int) (gray*scale[band] + off[band] + 0.5);
                CLAMP(g);
            }
            int tmp = g*step;
            int frac = (int)(tmp & 0xFF);
            int bin = tmp >> 8;
            Xil_unsigned8 lowVal = bin * delta;
            Xil_unsigned8 highVal = lowVal + delta;
            Xil_unsigned8* pDmat = pDmatBand;
            Xil_unsigned8* pDith = pDithBand + gray;
            for(unsigned int dcount=0; dcount<dmat_sz; dcount++) {
                int threshold = *pDmat++;
                if(frac > threshold) {
                    *pDith = highVal;
                } else {
                    *pDith = lowVal;
                }
                pDith += 256;
            } // end dithermask entry
        } // end gray level
    } // end band

    isOKFlag = TRUE;
    return;
}

XiliOrderedDitherLut::~XiliOrderedDitherLut()
{
    delete []multi_table;
}

int XiliOrderedDitherLut::isOK()
{
    if(this == NULL) {
        return FALSE;
    } else {
        if(isOKFlag == TRUE) {
            return TRUE;
        } else {
            delete this;
            return FALSE;
        }
    }
}


void
XiliOrderedDitherLut::dither8(Xil_unsigned8* src_scanline,
                              unsigned int   src_pixel_stride,
                              unsigned int   src_scanline_stride,
                              Xil_unsigned8* dst_scanline,
                              unsigned int   dst_pixel_stride,
                              unsigned int   dst_scanline_stride,
                              unsigned int   width,
                              unsigned int   height,
                              unsigned int   xpos,
                              unsigned int   ypos)
{
    ODCase this_case = special_case;

    unsigned int xmod = xpos % dmat_w;
    unsigned int ymod = ypos % dmat_h;

    // Look for the special ALIGNED case
    // Must be:
    //   3 band source image
    //   dst_pixel_stride = 1
    //   dither mask width is multiple of 4
    //   dst_scanline_stride is multiple of 4
    //   dst_pixel address at dither zero position is a multiple of 4
    //
    if( (nbands == 3)           &&
        (dst_pixel_stride == 1) &&
        (xmod == 0)             &&
        (width >= dmat_w)       &&
        ((dmat_w % 4) == 0)   &&
        ((width % dmat_w) == 0) &&
        (((unsigned int)dst_scanline % 4) == 0) &&
        ((dst_scanline_stride % 4) == 0) ) {

        this_case = OD_3_BAND_ALIGNED;
    }

    for(int lineCount=height; lineCount!=0; lineCount--) {

        switch(this_case) {

          case OD_3_BAND_ALIGNED:
            Dither3BandAligned(src_scanline, src_pixel_stride,
                               dst_scanline,
                               width, xmod, ymod);
            break;

          case OD_3_BAND:
            Dither3Band(src_scanline, src_pixel_stride,
                        dst_scanline, dst_pixel_stride,
                        width, xmod, ymod);
            break;

          case OD_N_BAND:
            DitherNBand(src_scanline, src_pixel_stride,
                        dst_scanline, dst_pixel_stride,
                        width, xmod, ymod);
            break;

          default: // OD_GENERAL
            DitherNonTable(src_scanline, src_pixel_stride,
                           dst_scanline, dst_pixel_stride,
                           width, xmod, ymod);
            break;

        }

        ymod = (ymod + 1) % dmat_h;
        src_scanline += src_scanline_stride;
        dst_scanline += dst_scanline_stride;
    }
}

void
XiliOrderedDitherLut::cvtCellB(Xil_unsigned8  y0,
                               Xil_unsigned8  y1,
                               Xil_unsigned8  u,
                               Xil_unsigned8  v,
                               Xil_unsigned16 mask,
                               Xil_unsigned8* dst,
                               unsigned int   dst_ss)
{
    //
    // Pointers to the Y dither table
    // The addresses of the U and V dither tables are 
    // handled by using the offsets of 4096 for U and 8192 for V.
    // Since U and V are the same for the whole block, the 
    // table displacements are folded into the U and V values.
    //
    Xil_unsigned8* pDithY = dlut;
    unsigned int uu = u + 4096;
    unsigned int vv = v + 8192;

    //
    // Shift the 16 bit mask to the MSB of a 32 bit word.
    // This lets us just test the MSB and then shift to get the next value
    //
    unsigned int   mask32   = mask << 16;

    //
    // Do one row (4 pixels) per loop pass
    //
    unsigned int idx;
    unsigned int uv;
    for(int linecount=4; linecount!=0; linecount-- ) {
        //
        //
        //
        // Start at the zero X point of the mask.
        // Each dither position increments the dlut index by 256.
        // We put 4 values into an int before writing it back to memory.
        //
        // Test the MSB of the mask to decide which set of Y values to use
        // Cmap index value is sum of table entries for each band
        // 
        // Pixel 0
        //
        uv = pDithY[uu] + pDithY[vv];
        if(mask32 & 0x80000000) {
            idx = pDithY[y1] + uv;
        } else {
            idx = pDithY[y0] + uv;
        }
        mask32 <<= 1;
        pDithY += 256;


        // 
        // Pixel 1
        //
        uv = pDithY[uu] + pDithY[vv];
        if(mask32 & 0x80000000) {
            idx = (idx<<8) | (pDithY[y1] + uv);
        } else {
            idx = (idx<<8) | (pDithY[y0] + uv);
        }
        mask32 <<= 1;
        pDithY += 256;

        // 
        // Pixel 2
        //
        uv = pDithY[uu] + pDithY[vv];
        if(mask32 & 0x80000000) {
            idx = (idx<<8) | (pDithY[y1] + uv);
        } else {
            idx = (idx<<8) | (pDithY[y0] + uv);
        }
        mask32 <<= 1;
        pDithY += 256;

        // 
        // Pixel 3
        //
        uv = pDithY[uu] + pDithY[vv];
        if(mask32 & 0x80000000) {
            idx = (idx<<8) | (pDithY[y1] + uv);
        } else {
            idx = (idx<<8) | (pDithY[y0] + uv);
        }
        mask32 <<= 1;
        pDithY += 256;

        //
        // Add in the four identical cmap offset values
        //
        idx += cmap_base32;

#ifdef XIL_LITTLE_ENDIAN
        _XILI_BSWAP(idx);
#endif

        *((unsigned int*)dst) = idx;

        dst += dst_ss;
    }

}

     
//
// Special case for:
//   Dither mask width = multiple of 4
//   3 bands
//   Destination address at dmask zero position is 32 bit aligned
//   XIL_BYTE output
//
void
XiliOrderedDitherLut::Dither3BandAligned(Xil_unsigned8* src_pixel,
                                         unsigned int   src_pixel_stride,
                                         Xil_unsigned8* dst_pixel,
                                         unsigned int   width,
                                         unsigned int   xmod,
                                         unsigned int   ymod)
{
    Xil_unsigned8* pSrc       = src_pixel;
    Xil_unsigned8* pDst       = dst_pixel;
    unsigned int   src_ps     = src_pixel_stride;
    unsigned int   cycles4    = dmat_w / 4;
    unsigned int   base       = cmap_offset;
    unsigned int   idx;

    //
    // Pointers to each band's dither table for this Y position
    //
    Xil_unsigned8* dlutRow0 = dlut + ymod*dlut_row_stride;
    Xil_unsigned8* dlutRow1 = dlutRow0 + dlut_band_stride;
    Xil_unsigned8* dlutRow2 = dlutRow1 + dlut_band_stride;
    Xil_unsigned8* dlutCol0;
    Xil_unsigned8* dlutCol1;
    Xil_unsigned8* dlutCol2;

    unsigned int initialCount = (dmat_w - xmod) % dmat_w;
    unsigned int mainCycles   = (width - initialCount) / dmat_w;
    unsigned int finalCount   = (width - initialCount - mainCycles*dmat_w);

    if(initialCount > 0) {
        //
        // Start out at the actual x position (mod the dmat width)
        // Thus allows a simple lookup for each gray level
        //
        unsigned int xdelta = xmod*dlut_col_stride;
        dlutCol0 = dlutRow0 + xdelta;
        dlutCol1 = dlutRow1 + xdelta;
        dlutCol2 = dlutRow2 + xdelta;

        for(unsigned int count=initialCount; count!=0; count--) { 
            idx = dlutCol0[pSrc[0]] + dlutCol1[pSrc[1]] + dlutCol2[pSrc[2]];
            *pDst++ = idx + base;

            pSrc += src_ps;

            dlutCol0 += 256;
            dlutCol1 += 256;
            dlutCol2 += 256;
        }
    }
    xmod = (xmod + initialCount) % dmat_w;

    
    if(mainCycles > 0) {
        //
        // Do the main part - repeated cycles of the full dithermask width.
        // This starts at the zero X point of the mask.
        // We put 4 values into an int before writing it back to memory.
        //
        unsigned int* pDst32 = (unsigned int*)pDst;

        //
        // Construct a 32 bit word with 4 copies of the cmap offset,
        // one in each byte. Since the cmap index can never exceed 255,
        // we are guaranteed that overflow cannot occur. This lets us
        // add the offset to four pixels at a time. (Poor man's VIS).
        //
        unsigned int base32 = (cmap_offset<<8) | cmap_offset;
        base32 = (base32<<16) | base32;

        for(int count=mainCycles; count!=0; count--) {
            dlutCol0 = dlutRow0;
            dlutCol1 = dlutRow1;
            dlutCol2 = dlutRow2;
            for(unsigned int mult4=cycles4; mult4!=0; mult4--) {
                //
                // Cmap index value is sum of table entries for each band
                // 
                //
                idx = dlutCol0[pSrc[0]] + dlutCol1[pSrc[1]] + 
                      dlutCol2[pSrc[2]];

                //
                // Increment dither table ptrs to next dither position
                //
                dlutCol0 += 256; dlutCol1 += 256; dlutCol2 += 256;
                pSrc += src_ps;

                //
                // Pack four cmap entries into the 32 bit word.
                //
                idx = (idx<<8) | (dlutCol0[pSrc[0]] + dlutCol1[pSrc[1]] + 
                                  dlutCol2[pSrc[2]]);
                dlutCol0 += 256; dlutCol1 += 256; dlutCol2 += 256;
                pSrc += src_ps;

                idx = (idx<<8) | (dlutCol0[pSrc[0]] + dlutCol1[pSrc[1]] + 
                                  dlutCol2[pSrc[2]]);
                dlutCol0 += 256; dlutCol1 += 256; dlutCol2 += 256;
                pSrc += src_ps;

                idx = (idx<<8) | (dlutCol0[pSrc[0]] + dlutCol1[pSrc[1]] + 
                                  dlutCol2[pSrc[2]]);
                dlutCol0 += 256; dlutCol1 += 256; dlutCol2 += 256;
                pSrc += src_ps;

                //
                // Add in the four identical cmap offset values
                //
                idx += base32;

#ifdef XIL_LITTLE_ENDIAN
                _XILI_BSWAP(idx);
#endif

                *pDst32++ = idx;
            }
        }
    }
    xmod = (xmod + mainCycles*dmat_w) % dmat_w;


    //
    // Do the final section, if needed
    //
    if(finalCount > 0) {
        //
        // Start out at the actual x position (mod the dmat width)
        // Thus allows a simple lookup for each gray level
        //
        unsigned int xdelta = xmod*dlut_col_stride;
        dlutCol0 = dlutRow0 + xdelta;
        dlutCol1 = dlutRow1 + xdelta;
        dlutCol2 = dlutRow2 + xdelta;

        for(unsigned int count=finalCount; count!=0; count--) { 
            idx = dlutCol0[pSrc[0]] + dlutCol1[pSrc[1]] + dlutCol2[pSrc[2]];
            *pDst++ = idx + base;

            pSrc += src_ps;

            dlutCol0 += 256;
            dlutCol1 += 256;
            dlutCol2 += 256;
        }
    }
        
    return;
}

//
// Special case for:
//   3 bands
//   Any Dmask size <= 16x16
//   No alignment restrictions
//   XIL_BYTE output
//
void
XiliOrderedDitherLut::Dither3Band(Xil_unsigned8* src_pixel,
                                  unsigned int   src_pixel_stride,
                                  Xil_unsigned8* dst_pixel,
                                  unsigned int   dst_pixel_stride,
                                  unsigned int   width,
                                  unsigned int   xmod,
                                  unsigned int   ymod)
{
    Xil_unsigned8* pSrc       = src_pixel;
    Xil_unsigned8* pDst       = dst_pixel;
    unsigned int   src_ps     = src_pixel_stride;
    unsigned int   dst_ps     = dst_pixel_stride;
    int            base       = cmap_offset;

    //
    // Pointers to each band's dither table for this Y position
    // Thus allows a simple lookup for each gray level
    //
    Xil_unsigned8* dlut0 = dlut + ymod*dlut_row_stride;
    Xil_unsigned8* dlut1 = dlut0 + dlut_band_stride;
    Xil_unsigned8* dlut2 = dlut1 + dlut_band_stride;

    //
    // Save limit of one step beyond X dither size of loop control
    //
    Xil_unsigned8* dlutLimit = dlut0 + dmat_w*dlut_col_stride;

    //
    // Start out at the actual x position (mod the dmat width)
    //
    unsigned int xdelta = xmod*dlut_col_stride;
    Xil_unsigned8* pDtab0 = dlut0 + xdelta;
    Xil_unsigned8* pDtab1 = dlut1 + xdelta;
    Xil_unsigned8* pDtab2 = dlut2 + xdelta;

    for(unsigned int count=width; count!=0; count--) {
        int idx = pDtab0[pSrc[0]] + pDtab1[pSrc[1]] + pDtab2[pSrc[2]];
        *pDst = (Xil_unsigned8) (idx + base);

        pSrc += src_ps;
        pDst += dst_ps;

        //
        // Step to next dither column (for first band only)
        //
        pDtab0 += 256;

        //
        // Test if we wrap around back to the zero X dither position
        //
        if(pDtab0 >= dlutLimit) {
            // 
            // Reset to dither X origin for this Y
            //
            pDtab0 = dlut0;
            pDtab1 = dlut1;
            pDtab2 = dlut2;
        } else {
            //
            // Continue normally; increment the other two pointers
            //
            pDtab1 += 256;
            pDtab2 += 256;
        }
    }

    return;
}
//
// Special case for:
//   N bands (up to _XIL_OD_MAX_BANDS)
//   Table size within limits
//   Any alignment
//   XIL_BYTE output
//
void
XiliOrderedDitherLut::DitherNBand(Xil_unsigned8* src_pixel,
                                  unsigned int   src_pixel_stride,
                                  Xil_unsigned8* dst_pixel,
                                  unsigned int   dst_pixel_stride,
                                  unsigned int   width,
                                  unsigned int   xmod,
                                  unsigned int   ymod)
{
    Xil_unsigned8* pSrc       = src_pixel;
    Xil_unsigned8* pDst       = dst_pixel;
    unsigned int   src_ps     = src_pixel_stride;
    unsigned int   dst_ps     = dst_pixel_stride;
    int            base       = cmap_offset;

    Xil_unsigned8* dlutRow = dlut + ymod*dlut_row_stride;

    //
    // Save limit of one step beyond X dither size of loop control
    //
    Xil_unsigned8* dlutLimit = dlutRow + dmat_w*dlut_col_stride;

    //
    // Start out at the actual x position (mod the dmat width)
    //
    Xil_unsigned8* dlutCol = dlutRow + xmod*dlut_col_stride;

    for(unsigned int count=width; count!=0; count--) {
        int idx = base;
        Xil_unsigned8* dlutBand = dlutCol;
        for(unsigned int i=0; i<nbands; i++) {
            idx += dlutBand[*(pSrc+i)];
            dlutBand += dlut_band_stride;
        }

        *pDst = idx;

        pSrc += src_ps;
        pDst += dst_ps;

        //
        // Next dither column
        //
        dlutCol += 256;

        //
        // Test if we wrap around back to the zero X dither position
        //
        if(dlutCol >= dlutLimit) {
            // 
            // Reset to dither X origin for this Y
            //
            dlutCol = dlutRow;
        }
    }

    return;
}

void
XiliOrderedDitherLut::DitherNonTable(Xil_unsigned8* src_pixel,
                                     unsigned int   src_pixel_stride,
                                     Xil_unsigned8* dst_pixel,
                                     unsigned int   dst_pixel_stride,
                                     unsigned int   width,
                                     unsigned int   xmod,
                                     unsigned int   ymod)
{
    Xil_unsigned8* dmatYBase = &dmat[dmat_w*ymod];
    Xil_unsigned8* dmatLimit = dmatYBase + dmat_w;

    Xil_unsigned8* pDmat = dmatYBase + xmod;
    for(int count=width; count!=0; count--) {
        Xil_unsigned8* src_band = src_pixel;
        Xil_unsigned8* pDmat_band = pDmat;

        int cmap_index = cmap_offset;
        for(unsigned int band=0; band<nbands; band++) {
            int tmp = (int)(*src_band++) * dims[band];
            int frac = (int)(tmp & 0xff);
            tmp >>= 8;
            if(frac > *pDmat_band) {
                tmp++;
            }
            pDmat_band += dmat_sz;

            cmap_index += tmp * mults[band];
        }
        *dst_pixel = (Xil_unsigned8)cmap_index;

        // Move to the next pixel
        src_pixel += src_pixel_stride;
        dst_pixel += dst_pixel_stride;
        pDmat++;

        //
        // Check if we need to restart at the x=0 point again.
        // Doing it this way avoids the modulus op.
        //
        if(pDmat >= dmatLimit) {
            pDmat = dmatYBase;
        }
    }

}

void
XiliOrderedDitherLut::dither8General(XilStorage* src_storage,
                                     Xil_unsigned8* dst_data,
                                     unsigned int   dst_pixel_stride,
                                     unsigned int   dst_scanline_stride,
                                     unsigned int xsize,
                                     unsigned int ysize,
                                     int abs_x1,
                                     int abs_y1,
                                     int box_offsetx,
                                     int box_offsety)
{

    Xil_unsigned8* dst_scanline = dst_data;
    for(unsigned int y=abs_y1; y<abs_y1+ysize; y++) {
        unsigned int ymod = y % dmat_h;
        Xil_unsigned8* dmatYBase = &dmat[dmat_w*ymod];
        Xil_unsigned8* dmatLimit = dmatYBase + dmat_w;

        unsigned int xmod = abs_x1 % dmat_w;
        Xil_unsigned8* pDmat = dmatYBase + xmod;
        Xil_unsigned8* dst_pixel = dst_scanline;
        for(unsigned int x=abs_x1; x<abs_x1+xsize; x++) {
            Xil_unsigned8* pDmat_band = pDmat;

            int cmap_index = cmap_offset;
            for(unsigned int band=0; band<nbands; band++) {
                //
                // Locate the band of the current pixel
                //
                unsigned int   src_pixel_stride;
                unsigned int   src_scanline_stride;
                Xil_unsigned8* src_data;
                src_storage->getStorageInfo(band,
                                           &src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL,
                                           (void**)&src_data);
                Xil_unsigned8* src_band = src_data +
                                          (y-box_offsety) * src_scanline_stride +
                                          (x-box_offsetx) * src_pixel_stride;

                int tmp = (int)(*src_band) * dims[band];
                int frac = (int)(tmp & 0xff);
                tmp >>= 8;
                if(frac > *pDmat_band) {
                    tmp++;
                }
                pDmat_band += dmat_sz;

                cmap_index += tmp * mults[band];
            }
            *dst_pixel = (Xil_unsigned8)cmap_index;

            // Move to the next pixel
            dst_pixel += dst_pixel_stride;
            pDmat++;

            //
            // Check if we need to restart at the x=0 point again.
            // Doing it this way avoids the modulus op.
            //
            if(pDmat >= dmatLimit) {
                pDmat = dmatYBase;
            }
        }

        dst_scanline += dst_scanline_stride;

    }

}

Xil_unsigned8*
XiliOrderedDitherLut::getYDitherLut()
{
    return yDitherLut;
}

unsigned int*
XiliOrderedDitherLut::getUDitherLut()
{
    return uvDitherLut;
}

unsigned int*
XiliOrderedDitherLut::getVDitherLut()
{
    return uvDitherLut + 2*2*NGRAY;
}


//
// Routine to dither an 8x8 block of 4:1:1 YUV data .
// This assumes that a 4x4 dither matrix is in use and
// an 855 colorcube has been specified.
//
void
XiliOrderedDitherLut::dither411(Xil_unsigned8* ysrc,
                                Xil_unsigned8* usrc,
                                Xil_unsigned8* vsrc,
                                unsigned int   y_ss,
                                unsigned int   u_ss,
                                unsigned int   v_ss,
                                Xil_unsigned8* dst,
                                unsigned int      ,  // dst_ps
                                unsigned int   dst_ss)
{
    Xil_unsigned8* ylut = getYDitherLut();
    unsigned int*  ulut = getUDitherLut();
    unsigned int*  vlut = getVDitherLut();

    unsigned int* dst32 = (unsigned int*)dst;
    unsigned int  dst32_ss = dst_ss >> 2;

    //
    // Construct a 32 bit word with 4 copies of the cmap offset,
    // one in each byte. Since the cmap index can never exceed 255,
    // we are guaranteed that overflow cannot occur. This lets us
    // add the offset to four pixels at a time. (Poor man's VIS).
    //
    unsigned int base32 = (cmap_offset<<8) | cmap_offset;
    base32 = (base32<<16) | base32;

    Xil_unsigned8* pYLine = ysrc;
    Xil_unsigned8* pULine = usrc;
    Xil_unsigned8* pVLine = vsrc;
    unsigned int idx;
    unsigned int ycomp;
    unsigned int* pDst32 = dst32;

    for(int fourlines=0; fourlines<2; fourlines++) {
        //
        // Set up at origin of 4x4 dither masks.
        // The uv masks are equivalent to 2x2 due to the subsampling
        //
        Xil_unsigned8* pYLut = ylut;
        unsigned int*  pULut = ulut;
        unsigned int*  pVLut = vlut;

        for(int linepair=0; linepair<2; linepair++) {

            //
            // Get the 4 U and V src values for this line pair
            // and lookup their contributon to the dither index
            //
            unsigned int u0 = pULut[pULine[0]];
            unsigned int u1 = pULut[NGRAY+pULine[1]];
            unsigned int u2 = pULut[pULine[2]];
            unsigned int u3 = pULut[NGRAY+pULine[3]];

            unsigned int v0 = pVLut[pVLine[0]];
            unsigned int v1 = pVLut[NGRAY+pVLine[1]];
            unsigned int v2 = pVLut[pVLine[2]];
            unsigned int v3 = pVLut[NGRAY+pVLine[3]];

            //
            // Upper left word
            //
            idx = base32;

            //
            // Pack 4 Y dither components into an int
            //
            ycomp = pYLut[pYLine[0]];
            ycomp = (ycomp<<8) + pYLut[NGRAY+pYLine[1]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*2+pYLine[2]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*3+pYLine[3]];
            idx += ycomp;

            //
            // Add in 4 U and V dither components into ints
            //
            idx += (u0 & 0xffff0000) | (u1 >> 16);
            idx += (v0 & 0xffff0000) | (v1 >> 16);

            //
            // Combine the components and
            // Write out the 4 packed dithered pixels
            //
#ifdef XIL_LITTLE_ENDIAN
            _XILI_BSWAP(idx);
#endif
            pDst32[0] = idx;

            //
            // Upper right word
            //
            idx = base32;

            //
            // Pack 4 Y dither components into an int
            //
            ycomp = pYLut[pYLine[4]];
            ycomp = (ycomp<<8) + pYLut[NGRAY+pYLine[5]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*2+pYLine[6]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*3+pYLine[7]];
            idx += ycomp;

            //
            // Pack 4 U and V dither components into ints
            //
            idx += (u2 & 0xffff0000) | (u3 >> 16);
            idx += (v2 & 0xffff0000) | (v3 >> 16);

            //
            // Combine the components and
            // Write out the 4 packed dithered pixels
            //
#ifdef XIL_LITTLE_ENDIAN
            _XILI_BSWAP(idx);
#endif
            pDst32[1] = idx;

            pDst32 += dst32_ss;
            pYLine += y_ss;
            pYLut  += 4*NGRAY;

            //
            // Lower left word
            //
            idx = base32;

            //
            // Pack 4 Y dither components into an int
            //
            ycomp = pYLut[pYLine[0]];
            ycomp = (ycomp<<8) + pYLut[NGRAY+pYLine[1]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*2+pYLine[2]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*3+pYLine[3]];
            idx += ycomp;

            //
            // Pack 4 U and V dither components into ints
            //
            idx += (u0 << 16) | (u1 & 0xffff);
            idx += (v0 << 16) | (v1 & 0xffff);

            //
            // Combine the components and
            // Write out the 4 packed dithered pixels
            //
#ifdef XIL_LITTLE_ENDIAN
            _XILI_BSWAP(idx);
#endif
            pDst32[0] = idx;

            //
            // Lower right word
            //
            idx = base32;

            //
            // Pack 4 Y dither components into an int
            //
            ycomp = pYLut[pYLine[4]];
            ycomp = (ycomp<<8) + pYLut[NGRAY+pYLine[5]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*2+pYLine[6]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*3+pYLine[7]];
            idx += ycomp;

            //
            // Pack 4 U and V dither components into ints
            //
            idx += (u2 << 16) | (u3 & 0xffff);
            idx += (v2 << 16) | (v3 & 0xffff);

            //
            // Combine the components and
            // Write out the 4 packed dithered pixels
            //
#ifdef XIL_LITTLE_ENDIAN
            _XILI_BSWAP(idx);
#endif
            pDst32[1] = idx;

            pDst32 += dst32_ss;
            pYLine += y_ss;
            pYLut  += 4*NGRAY;

            //
            // Advance uv src and dither lut pointers for next line pair
            //
            pULine += u_ss;
            pVLine += v_ss;
            pULut  += 2*NGRAY;
            pVLut  += 2*NGRAY;
        }


    }

    return;
}



//
// SHORT version (This is used for Jpeg and Mpeg1)
// Routine to dither an 8x8 block of 4:1:1 YUV data .
// This assumes that a 4x4 dither matrix is in use and
// an 855 colorcube has been specified.
//
void
XiliOrderedDitherLut::dither411(Xil_signed16*  ysrc,
                                Xil_signed16*  usrc,
                                Xil_signed16*  vsrc,
                                unsigned int   y_ss,
                                unsigned int   u_ss,
                                unsigned int   v_ss,
                                Xil_unsigned8* dst,
                                unsigned int      ,  // dst_ps
                                unsigned int   dst_ss)
{
    Xil_unsigned8* ylut = getYDitherLut();
    unsigned int*  ulut = getUDitherLut();
    unsigned int*  vlut = getVDitherLut();

    unsigned int* dst32 = (unsigned int*)dst;
    unsigned int  dst32_ss = dst_ss >> 2;

    //
    // Construct a 32 bit word with 4 copies of the cmap offset,
    // one in each byte. Since the cmap index can never exceed 255,
    // we are guaranteed that overflow cannot occur. This lets us
    // add the offset to four pixels at a time. (Poor man's VIS).
    //
    unsigned int base32 = (cmap_offset<<8) | cmap_offset;
    base32 = (base32<<16) | base32;

    Xil_signed16*  pYLine = ysrc;
    Xil_signed16*  pULine = usrc;
    Xil_signed16*  pVLine = vsrc;
    unsigned int idx;
    unsigned int ycomp;
    unsigned int* pDst32 = dst32;

    for(int fourlines=0; fourlines<2; fourlines++) {
        //
        // Set up at origin of 4x4 dither masks.
        // The uv masks are equivalent to 2x2 due to the subsampling
        //
        Xil_unsigned8* pYLut = ylut;
        unsigned int*  pULut = ulut;
        unsigned int*  pVLut = vlut;

        for(int linepair=0; linepair<2; linepair++) {

            //
            // Get the 4 U and V src values for this line pair
            // and lookup their contributon to the dither index
            //
            unsigned int u0 = pULut[pULine[0]];
            unsigned int u1 = pULut[NGRAY+pULine[1]];
            unsigned int u2 = pULut[pULine[2]];
            unsigned int u3 = pULut[NGRAY+pULine[3]];

            unsigned int v0 = pVLut[pVLine[0]];
            unsigned int v1 = pVLut[NGRAY+pVLine[1]];
            unsigned int v2 = pVLut[pVLine[2]];
            unsigned int v3 = pVLut[NGRAY+pVLine[3]];

            //
            // Upper left word
            //
            idx = base32;

            //
            // Pack 4 Y dither components into an int
            //
            ycomp = pYLut[pYLine[0]];
            ycomp = (ycomp<<8) + pYLut[NGRAY+pYLine[1]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*2+pYLine[2]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*3+pYLine[3]];
            idx += ycomp;

            //
            // Add in 4 U and V dither components into ints
            //
            idx += (u0 & 0xffff0000) | (u1 >> 16);
            idx += (v0 & 0xffff0000) | (v1 >> 16);

            //
            // Combine the components and
            // Write out the 4 packed dithered pixels
            //
#ifdef XIL_LITTLE_ENDIAN
            _XILI_BSWAP(idx);
#endif
            pDst32[0] = idx;

            //
            // Upper right word
            //
            idx = base32;

            //
            // Pack 4 Y dither components into an int
            //
            ycomp = pYLut[pYLine[4]];
            ycomp = (ycomp<<8) + pYLut[NGRAY+pYLine[5]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*2+pYLine[6]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*3+pYLine[7]];
            idx += ycomp;

            //
            // Pack 4 U and V dither components into ints
            //
            idx += (u2 & 0xffff0000) | (u3 >> 16);
            idx += (v2 & 0xffff0000) | (v3 >> 16);

            //
            // Combine the components and
            // Write out the 4 packed dithered pixels
            //
#ifdef XIL_LITTLE_ENDIAN
            _XILI_BSWAP(idx);
#endif
            pDst32[1] = idx;

            pDst32 += dst32_ss;
            pYLine += y_ss;
            pYLut  += 4*NGRAY;

            //
            // Lower left word
            //
            idx = base32;

            //
            // Pack 4 Y dither components into an int
            //
            ycomp = pYLut[pYLine[0]];
            ycomp = (ycomp<<8) + pYLut[NGRAY+pYLine[1]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*2+pYLine[2]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*3+pYLine[3]];
            idx += ycomp;

            //
            // Pack 4 U and V dither components into ints
            //
            idx += (u0 << 16) | (u1 & 0xffff);
            idx += (v0 << 16) | (v1 & 0xffff);

            //
            // Combine the components and
            // Write out the 4 packed dithered pixels
            //
#ifdef XIL_LITTLE_ENDIAN
            _XILI_BSWAP(idx);
#endif
            pDst32[0] = idx;

            //
            // Lower right word
            //
            idx = base32;

            //
            // Pack 4 Y dither components into an int
            //
            ycomp = pYLut[pYLine[4]];
            ycomp = (ycomp<<8) + pYLut[NGRAY+pYLine[5]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*2+pYLine[6]];
            ycomp = (ycomp<<8) + pYLut[NGRAY*3+pYLine[7]];
            idx += ycomp;

            //
            // Pack 4 U and V dither components into ints
            //
            idx += (u2 << 16) | (u3 & 0xffff);
            idx += (v2 << 16) | (v3 & 0xffff);

            //
            // Combine the components and
            // Write out the 4 packed dithered pixels
            //
#ifdef XIL_LITTLE_ENDIAN
            _XILI_BSWAP(idx);
#endif
            pDst32[1] = idx;

            pDst32 += dst32_ss;
            pYLine += y_ss;
            pYLut  += 4*NGRAY;

            //
            // Advance uv src and dither lut pointers for next line pair
            //
            pULine += u_ss;
            pVLine += v_ss;
            pULut  += 2*NGRAY;
            pVLut  += 2*NGRAY;
        }


    }

    return;
}

void
XiliOrderedDitherLut::cvtMacroBlock(Xil_unsigned8* ysrc,
                                    Xil_unsigned8* usrc,
                                    Xil_unsigned8* vsrc,
                                    unsigned int   y_ss,
                                    unsigned int   u_ss,
                                    unsigned int   v_ss,
                                    Xil_unsigned8* dst,
                                    unsigned int   dst_ps,
                                    unsigned int   dst_ss)
{
    //
    // Upper left
    // 
    dither411(ysrc, usrc, vsrc,
              y_ss, u_ss, v_ss,
              dst, dst_ps, dst_ss);

    //
    // Upper right
    // 
    dither411(ysrc+8, usrc+4, vsrc+4,
              y_ss, u_ss, v_ss,
              dst+8, dst_ps, dst_ss);

    //
    // Lower left
    // 
    dither411(ysrc+8*y_ss, usrc+4*u_ss, vsrc+4*v_ss,
              y_ss, u_ss, v_ss,
              dst+8*dst_ss, dst_ps, dst_ss);

    //
    // Lower right
    // 
    dither411(ysrc+8*y_ss+8, usrc+4*u_ss+4, vsrc+4*v_ss+4,
              y_ss, u_ss, v_ss,
              dst+8*dst_ss+8, dst_ps, dst_ss);

}

void
XiliOrderedDitherLut::cvtMacroBlock(Xil_signed16*  ysrc,
                                    Xil_signed16*  usrc,
                                    Xil_signed16*  vsrc,
                                    unsigned int   y_ss,
                                    unsigned int   u_ss,
                                    unsigned int   v_ss,
                                    Xil_unsigned8* dst,
                                    unsigned int   dst_ps,
                                    unsigned int   dst_ss)
{
    //
    // Upper left
    // 
    dither411(ysrc, usrc, vsrc,
              y_ss, u_ss, v_ss,
              dst, dst_ps, dst_ss);

    //
    // Upper right
    // 
    dither411(ysrc+8, usrc+4, vsrc+4,
              y_ss, u_ss, v_ss,
              dst+8, dst_ps, dst_ss);

    //
    // Lower left
    // 
    dither411(ysrc+8*y_ss, usrc+4*u_ss, vsrc+4*v_ss,
              y_ss, u_ss, v_ss,
              dst+8*dst_ss, dst_ps, dst_ss);

    //
    // Lower right
    // 
    dither411(ysrc+8*y_ss+8, usrc+4*u_ss+4, vsrc+4*v_ss+4,
              y_ss, u_ss, v_ss,
              dst+8*dst_ss+8, dst_ps, dst_ss);

}


