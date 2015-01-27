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
//  File:   CellBDecompressorData.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:15:29, 03/10/00
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
//  MT-level:  <??????>
//
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)CellBDecompressorData.cc	1.4\t00/03/10  "

#include <xil/xilGPI.hh>
#include "CellBManagerCompressorData.hh"
#include "XilDeviceCompressionCellB.hh"

//------------------------------------------------------------------------
//
//  Functions:
//    CellBDecompressorData::CellBDecompressorData
//    CellBDecompressorData::~CellBDecompressorData
//    CellBDecompressorData::initValues
//    CellBDecompressorData::deleteValues
//    CellBDecompressorData::reset
//  Created:    92/11/06
//
//  Description:
//    
//    reset() is called when an xil_cis_reset occurs
//    initValues() is the common code between the constructor and reset()
//    deleteValues() is the common code between the destructor and reset()
//
//------------------------------------------------------------------------

CellBDecompressorData::CellBDecompressorData (
       CellBManagerCompressorData* ct) 
{
    // These two tables are initialized only once.  They never change.
    yytable = ct->yytable;
    uvtable = ct->uvtable;

    initValues();
}

CellBDecompressorData::~CellBDecompressorData(void)
{
    deleteValues();
}

void
CellBDecompressorData::initValues(void)
{
    isOKFlag = FALSE;

    // make sure it is safe to call the destructor by setting to NULL all
    // the pointers that are used with 'delete'
    histImageEnum h;
    for (h=I8; h < HISTIMAGEMAX; h = (histImageEnum)(h + 1)) {
      histImages[h] = NULL;
    }
    cellBFrame = NULL;
    ignoreHistory = FALSE;

    isOKFlag = TRUE;
}

void
CellBDecompressorData::deleteValues(void)
{
    histImageEnum h;
    for (h=I8; h < HISTIMAGEMAX; h = (histImageEnum)(h + 1)) {
      delete histImages[h];
    }

    delete cellBFrame;
}

void
CellBDecompressorData::reset(void)
{
    deleteValues();
    initValues();
}

//------------------------------------------------------------------------
//
//  Function:    CellBDecompressorData::getCellBFrame()
//  Created:    92/11/30
//
//  Description:
//    
//    Return a pointer to CellB's 8-bit image history buffer
//    
//------------------------------------------------------------------------
CellBFrame*
CellBDecompressorData::getCellBFrame(unsigned int w, unsigned int h)
{
    if (cellBFrame == NULL) {
      // allocate a CellBFrame object, dividing the image width & height
      // by 4 since one cell = 4x4 pixels
      cellBFrame = new CellBFrame(w >> 2, h >> 2);
      // TODO???: cellBFrame = cellBFrame->ok();
      if (cellBFrame == NULL) {
        // secondary out of memory error
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", FALSE);
        // TODO: out-of-memory error? couldn't create tmp image error?
      }
      // TODO: cache w & h for future sanity checks?
    }
    return cellBFrame;
}

//------------------------------------------------------------------------
//
//  Function:    CellBDecompressorData::getHistImage()
//  Created:    93/4/21
//
//  Description:
//    
//    Return a pointer to CellB's image history buffer for the 
//    appropriate format
//    
//------------------------------------------------------------------------

#ifdef MOLECULE_SUPPORT

CellBHistoryImage*
CellBDecompressorData::getHistImage(histImageEnum imagenum,
                                    unsigned int w,
                                    unsigned int h,
                                    unsigned int f,
                                    unsigned int nbands, 
                                    unsigned int parent_bands,
                                    unsigned int band_offset)
{
    if ((histImages[imagenum] == NULL) ||
       ((imagenum == IAny) &&
        !histImages[imagenum]->verifyImage(w,
                                           h,
                                           nbands, 
                                           parent_bands, 
                                           band_offset))) {
      delete histImages[imagenum];
      histImages[imagenum] =
            new CellBHistoryImage(w,
                                  h,
                                  f,
                                  nbands,
                                  parent_bands, 
                                  band_offset);

      if ((histImages[imagenum] == NULL) || !histImages[imagenum]->ok()) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", FALSE);
        return NULL;
      }
    } else {
      histImages[imagenum]->updateImage(f);
    }
    return histImages[imagenum];
}

#endif


//------------------------------------------------------------------------
//
//  Function:    CellBDecompressorData::ditherInit()
//  Created:    92/11/16
//
//  Description:
//    
//    Initialize the CellB dither tables
//    
//------------------------------------------------------------------------
int
CellBDecompressorData::ditherInit(XilLookupColorcube* colorcube,
                                  XilDitherMask*      dithermask,
                                  float*              rescale,
                                  float*              offset)
{
    int i, j;

    Xil_unsigned8 ydither[MAX_BYTE_VAL][16];
    Xil_unsigned8 udither[MAX_BYTE_VAL][16];
    Xil_unsigned8 vdither[MAX_BYTE_VAL][16];

    const int*          mults = colorcube->getMultipliers();
    const unsigned int* dims  = colorcube->getDimensions();
    
    //
    // Check that the colorcube multiplier for the Y band is 1, because one
    // of the CellB dither optimizations depends on Y being the fastest varying
    // component of the colorcube.  Check that the other multipliers are
    // positive, since negative multipliers aren't currently supported.
    //
    if (mults[0] != 1 ||
        mults[1] <= 0 ||
        mults[2] <= 0 ) {
      return XIL_FAILURE;
    }

    // TODO: should we also check the dimensions?  Make sure they're greater
    // than 1?

    // get a pointer to the dithermask data (we checked for NULL before
    // calling this function)
    const float* dmask_vals = dithermask->getData();
    
    for (int band = 0; band < 3; band++) {
      Xil_unsigned8* ptr;
      int colorcube_offset;

      // The dithermask band order is YUV
      switch (band) {
        case 0:
          ptr = (Xil_unsigned8*)ydither;
          colorcube_offset = 0;
          break;

        case 1:
          ptr = (Xil_unsigned8*)udither;
          // the colorcube offset is accounted for in udither
          colorcube_offset = colorcube->getOffset();
          break;

        case 2:
          ptr = (Xil_unsigned8*)vdither;
          colorcube_offset = 0;
          break;
    }

    // calculate a conversion factor -- see below
    float convert = (dims[band] - 1) / 255.0;

    for (i = 0; i < MAX_BYTE_VAL; i++) {
        //
        // For each possible value, perform the xil_rescale operation
        // specified for this band and clamp the result to the
        // range 0.0 to 255.0
        //
        float val = i * rescale[band] + offset[band];
        if (val <   0.0) val =   0.0;
        if (val > 255.0) val = 255.0;

        //
        // Convert the value from the range [ 0..255 ] to the range
        // [ 0..dims[band]-1 ], and temporarily store the result
        // in 'fraction'
        //
        float fraction = val * convert;

        //
        // Now separate the result into whole and fractional parts, with
        // the whole part going into lo_val.
        //
        int lo_val = (int)fraction;
        fraction -= lo_val;

        //
        // hi_val will normally be 1 greater than lo_val, unless lo_val
        // is at the top of the range (which will be the case when
        // val was 255.0).
        //
        int hi_val = lo_val + 1;
        if (lo_val >= dims[band] - 1) {
          hi_val = lo_val;
        }

        //
        // Adjust lo_val & hi_val based on the multiplier for this band
        // and also the colorcube offset.
        //
        lo_val = (lo_val * mults[band]) + colorcube_offset;
        hi_val = (hi_val * mults[band]) + colorcube_offset;

        // the dithermask values are in [3][4][4] order
        const float* dmask_ptr = dmask_vals + (band << 4);

        //
        // Test the fraction against each value in the dithermask for
        // this band and assign the dither table element to hi_val
        // or lo_val accordingly.
        //
        for (j = 0; j < 16; j++) {
          if (fraction > dmask_ptr[j]) {
            ptr[j] = hi_val;
          } else {
            ptr[j] = lo_val;
          }
        }
        ptr += 16;
      }
    }


    for (i = 0; i < UVTABLE_SIZE; i++) {
      int u, v;

      v = uvtable[i];
      u = v >> 8;
      v = v & 255;

      // get pointers to the dither array for a given u & v
      Xil_unsigned8* uptr = &udither[u][0];
      Xil_unsigned8* vptr = &vdither[v][0];

      for (j = 0; j < 4; j++) {
        Xil_unsigned32 tmp;
        tmp  = *uptr++ + *vptr++;
        tmp <<= 8;
        tmp += *uptr++ + *vptr++;
        tmp <<= 8;
        tmp += *uptr++ + *vptr++;
        tmp <<= 8;
        tmp += *uptr++ + *vptr++;
        uvdither[i][j] = tmp;
      }
    }

    for (i = 0; i < MAX_BYTE_VAL; i++) {
      int y0, y1;

      y1 = yytable[i];
      y0 = y1 >> 8;
      y1 = y1 & 255;

      // get pointers to the dither array for a given y0 & y1
      Xil_unsigned8* y0ptr = &ydither[y0][0];
      Xil_unsigned8* y1ptr = &ydither[y1][0];

      for (j = 0; j < 4; j++) {
        Xil_unsigned32 tmp;
        //
        // The y0 values go in the most significant nibble of each
        // byte, while the y1 values in in the least significant nibble
        // of each byte.  This will facilitate some optimization in the
        // dither molecule.  In order for this trick to work, Y must be
        // the fastest varying component of the YUV colorcube, which we
        // check for elsewhere.
        //
        tmp  = (*y0ptr++ << 4) + *y1ptr++;
        tmp <<= 8;
        tmp += (*y0ptr++ << 4) + *y1ptr++;
        tmp <<= 8;
        tmp += (*y0ptr++ << 4) + *y1ptr++;
        tmp <<= 8;
        tmp += (*y0ptr++ << 4) + *y1ptr++;
        yydither[i][j] = tmp;
      }
    }
    return XIL_SUCCESS;
}
