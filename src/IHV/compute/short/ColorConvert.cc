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
//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:	ColorConvert.cc
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	10:12:15, 03/10/00
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
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)ColorConvert.cc	1.3\t00/03/10  "

#include "XilDeviceManagerComputeSHORT.hh"
#include "XiliCSop.hh"

//
// Forward declarations
//
void rgblinear_to_rgb709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgblinear_to_photoycc(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgblinear_to_ycc601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgblinear_to_ycc709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgblinear_to_cmy(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgblinear_to_cmyk(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgblinear_to_ylinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgblinear_to_y601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgblinear_to_y709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);

void rgb709_to_rgblinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgb709_to_photoycc(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgb709_to_ycc601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgb709_to_ycc709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgb709_to_cmy(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgb709_to_cmyk(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgb709_to_ylinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgb709_to_y601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void rgb709_to_y709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);

void photoycc_to_rgblinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void photoycc_to_rgb709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void photoycc_to_ycc601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void photoycc_to_ycc709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void photoycc_to_cmy(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void photoycc_to_cmyk(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void photoycc_to_ylinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void photoycc_to_y601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void photoycc_to_y709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);

void ycc601_to_rgblinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc601_to_rgb709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc601_to_photoycc(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc601_to_ycc709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc601_to_cmy(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc601_to_cmyk(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc601_to_ylinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc601_to_y601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc601_to_y709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);

void ycc709_to_rgblinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc709_to_rgb709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc709_to_photoycc(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc709_to_ycc601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc709_to_cmy(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc709_to_cmyk(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc709_to_ylinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc709_to_y601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ycc709_to_y709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);

void cmy_to_rgblinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmy_to_rgb709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmy_to_photoycc(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmy_to_ycc601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmy_to_ycc709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmy_to_cmyk(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmy_to_ylinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmy_to_y601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmy_to_y709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);

void cmyk_to_rgblinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmyk_to_rgb709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmyk_to_photoycc(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmyk_to_ycc601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmyk_to_ycc709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmyk_to_cmy(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmyk_to_ylinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmyk_to_y601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void cmyk_to_y709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);

void ylinear_to_rgblinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ylinear_to_rgb709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ylinear_to_photoycc(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ylinear_to_ycc601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ylinear_to_ycc709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ylinear_to_cmy(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ylinear_to_cmyk(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ylinear_to_y601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void ylinear_to_y709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);

void y601_to_rgblinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y601_to_rgb709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y601_to_photoycc(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y601_to_ycc601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y601_to_ycc709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y601_to_cmy(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y601_to_cmyk(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y601_to_ylinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y601_to_y709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);

void y709_to_rgblinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y709_to_rgb709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y709_to_photoycc(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y709_to_ycc601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y709_to_ycc709(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y709_to_cmy(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y709_to_cmyk(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y709_to_ylinear(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
void y709_to_y601(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);

//
// Opcode & operation table
//
typedef struct cs_op {
   int   opcode;  
   void  (*func_ptr)(Xil_signed16*, Xil_signed16*, unsigned int, unsigned int, unsigned int);
} CsOp;

CsOp CSoperation[] = {
	{ XIL_CS_RGBLINEAR_TO_rgb709,   rgblinear_to_rgb709},
	{ XIL_CS_RGBLINEAR_TO_photoycc, rgblinear_to_photoycc},
	{ XIL_CS_RGBLINEAR_TO_ycc601,   rgblinear_to_ycc601},
	{ XIL_CS_RGBLINEAR_TO_ycc709,   rgblinear_to_ycc709},
	{ XIL_CS_RGBLINEAR_TO_cmyk,     rgblinear_to_cmyk},
	{ XIL_CS_RGBLINEAR_TO_cmy,      rgblinear_to_cmy},
	{ XIL_CS_RGBLINEAR_TO_ylinear,  rgblinear_to_ylinear},
	{ XIL_CS_RGBLINEAR_TO_y601,     rgblinear_to_y601},
	{ XIL_CS_RGBLINEAR_TO_y709,     rgblinear_to_y709},

	{ XIL_CS_RGB709_TO_rgblinear, rgb709_to_rgblinear},
	{ XIL_CS_RGB709_TO_photoycc,  rgb709_to_photoycc},
	{ XIL_CS_RGB709_TO_ycc601,    rgb709_to_ycc601},
	{ XIL_CS_RGB709_TO_ycc709,    rgb709_to_ycc709},
	{ XIL_CS_RGB709_TO_cmyk,      rgb709_to_cmyk},
	{ XIL_CS_RGB709_TO_cmy,       rgb709_to_cmy},
	{ XIL_CS_RGB709_TO_ylinear,   rgb709_to_ylinear},
	{ XIL_CS_RGB709_TO_y601,      rgb709_to_y601},
	{ XIL_CS_RGB709_TO_y709,      rgb709_to_y709},

	{ XIL_CS_PHOTOYCC_TO_rgblinear, photoycc_to_rgblinear},
	{ XIL_CS_PHOTOYCC_TO_rgb709,    photoycc_to_rgb709},
	{ XIL_CS_PHOTOYCC_TO_ycc601,    photoycc_to_ycc601},
	{ XIL_CS_PHOTOYCC_TO_ycc709,    photoycc_to_ycc709},
	{ XIL_CS_PHOTOYCC_TO_cmyk,      photoycc_to_cmyk},
	{ XIL_CS_PHOTOYCC_TO_cmy,       photoycc_to_cmy},
	{ XIL_CS_PHOTOYCC_TO_ylinear,   photoycc_to_ylinear},
	{ XIL_CS_PHOTOYCC_TO_y601,      photoycc_to_y601},
	{ XIL_CS_PHOTOYCC_TO_y709,      photoycc_to_y709},

	{ XIL_CS_YCC601_TO_rgblinear, ycc601_to_rgblinear},
	{ XIL_CS_YCC601_TO_rgb709,    ycc601_to_rgb709},
	{ XIL_CS_YCC601_TO_photoycc,  ycc601_to_photoycc},
	{ XIL_CS_YCC601_TO_ycc709,    ycc601_to_ycc709},
	{ XIL_CS_YCC601_TO_cmyk,      ycc601_to_cmyk},
	{ XIL_CS_YCC601_TO_cmy,       ycc601_to_cmy},
	{ XIL_CS_YCC601_TO_ylinear,   ycc601_to_ylinear},
	{ XIL_CS_YCC601_TO_y601,      ycc601_to_y601},
	{ XIL_CS_YCC601_TO_y709,      ycc601_to_y709},

	{ XIL_CS_YCC709_TO_rgblinear, ycc709_to_rgblinear},
	{ XIL_CS_YCC709_TO_rgb709,    ycc709_to_rgb709},
	{ XIL_CS_YCC709_TO_photoycc,  ycc709_to_photoycc},
	{ XIL_CS_YCC709_TO_ycc601,    ycc709_to_ycc601},
	{ XIL_CS_YCC709_TO_cmyk,      ycc709_to_cmyk},
	{ XIL_CS_YCC709_TO_cmy,       ycc709_to_cmy},
	{ XIL_CS_YCC709_TO_ylinear,   ycc709_to_ylinear},
	{ XIL_CS_YCC709_TO_y601,      ycc709_to_y601},
	{ XIL_CS_YCC709_TO_y709,      ycc709_to_y709},

	{ XIL_CS_CMY_TO_rgblinear, cmy_to_rgblinear},
	{ XIL_CS_CMY_TO_rgb709,    cmy_to_rgb709},
	{ XIL_CS_CMY_TO_photoycc,  cmy_to_photoycc},
	{ XIL_CS_CMY_TO_ycc601,    cmy_to_ycc601},
	{ XIL_CS_CMY_TO_ycc709,    cmy_to_ycc709},
	{ XIL_CS_CMY_TO_cmyk,      cmy_to_cmyk},
	{ XIL_CS_CMY_TO_ylinear,   cmy_to_ylinear},
	{ XIL_CS_CMY_TO_y601,      cmy_to_y601},
	{ XIL_CS_CMY_TO_y709,      cmy_to_y709},

	{ XIL_CS_CMYK_TO_rgblinear, cmyk_to_rgblinear},
	{ XIL_CS_CMYK_TO_rgb709,    cmyk_to_rgb709},
	{ XIL_CS_CMYK_TO_photoycc,  cmyk_to_photoycc},
	{ XIL_CS_CMYK_TO_ycc601,    cmyk_to_ycc601},
	{ XIL_CS_CMYK_TO_ycc709,    cmyk_to_ycc709},
	{ XIL_CS_CMYK_TO_cmy,       cmyk_to_cmy},
	{ XIL_CS_CMYK_TO_ylinear,   cmyk_to_ylinear},
	{ XIL_CS_CMYK_TO_y601,      cmyk_to_y601},
	{ XIL_CS_CMYK_TO_y709,      cmyk_to_y709},

	{ XIL_CS_YLINEAR_TO_rgblinear, ylinear_to_rgblinear},
	{ XIL_CS_YLINEAR_TO_rgb709,    ylinear_to_rgb709},
	{ XIL_CS_YLINEAR_TO_photoycc,  ylinear_to_photoycc},
	{ XIL_CS_YLINEAR_TO_ycc601,    ylinear_to_ycc601},
	{ XIL_CS_YLINEAR_TO_ycc709,    ylinear_to_ycc709},
	{ XIL_CS_YLINEAR_TO_cmy,       ylinear_to_cmy},
	{ XIL_CS_YLINEAR_TO_cmyk,      ylinear_to_cmyk},
	{ XIL_CS_YLINEAR_TO_y601,      ylinear_to_y601},
	{ XIL_CS_YLINEAR_TO_y709,      ylinear_to_y709},

	{ XIL_CS_Y601_TO_rgblinear, y601_to_rgblinear},
	{ XIL_CS_Y601_TO_rgb709,    y601_to_rgb709},
	{ XIL_CS_Y601_TO_photoycc,  y601_to_photoycc},
	{ XIL_CS_Y601_TO_ycc601,    y601_to_ycc601},
	{ XIL_CS_Y601_TO_ycc709,    y601_to_ycc709},
	{ XIL_CS_Y601_TO_cmy,       y601_to_cmy},
	{ XIL_CS_Y601_TO_cmyk,      y601_to_cmyk},
	{ XIL_CS_Y601_TO_ylinear,   y601_to_ylinear},
	{ XIL_CS_Y601_TO_y709,      y601_to_y709},

	{ XIL_CS_Y709_TO_rgblinear, y709_to_rgblinear},
	{ XIL_CS_Y709_TO_rgb709,    y709_to_rgb709},
	{ XIL_CS_Y709_TO_photoycc,  y709_to_photoycc},
	{ XIL_CS_Y709_TO_ycc601,    y709_to_ycc601},
	{ XIL_CS_Y709_TO_ycc709,    y709_to_ycc709},
	{ XIL_CS_Y709_TO_cmy,       y709_to_cmy},
	{ XIL_CS_Y709_TO_cmyk,      y709_to_cmyk},
	{ XIL_CS_Y709_TO_ylinear,   y709_to_ylinear},
	{ XIL_CS_Y709_TO_y601,      y709_to_y601},

	{ 0, NULL}
};


XilStatus
XilDeviceManagerComputeSHORT::ColorConvert(XilOp*       op,
					                       unsigned     ,
					                       XilRoi*      roi,
					                       XilBoxList*  bl)
{
    //
    //  Split the list of XilBoxes to take tile boundaries into account.  This
    //  will work to ensure that no cobbling of the data is required because
    //  the boxes will not cross tile boundaries in the source images.
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

    XilColorspace*    src_cs;
    XilColorspace*    dest_cs;

    op->getParam(1, (void**)&src_cs);
    op->getParam(2, (void**)&dest_cs);

    unsigned int src_opcode = src_cs->getOpcode();
    unsigned int dest_opcode = dest_cs->getOpcode();

    //
    // Construct the Operation Code
    //
    unsigned int opcode = src_opcode << 16 | dest_opcode;

    int found = 0;

    for (int i = 0; CSoperation[i].opcode; i++) {
      if (opcode == CSoperation[i].opcode) {
        //
        // Found the appropriate function
        //
        found = 1;
        break;
      }
    }

    if (found == 0) {
      //
      // No opcode found, return failure
      //
      XIL_ERROR(dest->getSystemState(), XIL_ERROR_USER, "di-204", TRUE);
      return XIL_FAILURE;
    }

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dest_box;
    while(bl->getNext(&src_box, &dest_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
        XilStorage  src_storage(src);
        XilStorage  dest_storage(dest);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
                             XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dest->getStorage(&dest_storage, op, dest_box, "XilMemory",
                             XIL_WRITE_ONLY) == XIL_FAILURE)) {
            //
            //  Mark this box as failed and if that succeeds, continue
            //  processing the next box.  Otherwise, return XIL_FAILURE now.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
                return XIL_FAILURE;
            } else {
                continue;
            }
        }

        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (dest_storage.isType(XIL_PIXEL_SEQUENTIAL))) {

            unsigned int   src_pixel_stride;
            unsigned int   src_scanline_stride;
            Xil_signed16* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_signed16* dest_data;
            dest_storage.getStorageInfo(&dest_pixel_stride,
                                            &dest_scanline_stride,
                                            NULL, NULL,
                                            (void**)&dest_data);

            //
            //  Create a list of rectangles to loop over.  The resulting list
            //  of rectangles is the area left by intersecting the ROI with
            //  the destination box.
            //
            XilRectList    rl(roi, dest_box);

            int            x;
            int            y;
            unsigned int   xsize;
            unsigned int   ysize;

            while(rl.getNext(&x, &y, &xsize, &ysize)) {

                Xil_signed16* src_scanline = src_data +
                         (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_signed16* dest_scanline = dest_data +
                         (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_signed16* src_pixel = src_scanline;
                    Xil_signed16* dest_pixel = dest_scanline;

                    CSoperation[i].func_ptr(src_pixel,
                                            dest_pixel,
                                            xsize,
                                            src_pixel_stride,
                                            dest_pixel_stride);

                    //
                    // Move to next scanline
                    //
                    src_scanline += src_scanline_stride;
                    dest_scanline += dest_scanline_stride;

                } while (--ysize);
            }
        } else {
            //
            // General storage implementation
            //
            int b; // band index variable

            //
            // Get the number of bands in each image
            //
            unsigned int dest_nbands = dest->getNumBands();
            unsigned int src_nbands  = src->getNumBands();

            //
            // Set up band-indexed stride and data pointers.
            //
            Xil_signed16* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_signed16*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_signed16* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_signed16*)dest_storage.getDataPtr(b);
                dest_pixel_stride[b] = dest_storage.getPixelStride(b);
                dest_scanline_stride[b] = dest_storage.getScanlineStride(b);
            }

            //
            //  Create a list of rectangles to loop over.  The resulting list
            //  of rectangles is the area left by intersecting the ROI with
            //  the destination box.
            //
            XilRectList    rl(roi, dest_box);

            int            x;
            int            y;
            unsigned int   xsize;
            unsigned int   ysize;

            while(rl.getNext(&x, &y, &xsize, &ysize)) {

                int x2 = x + xsize;
                int y2 = y + ysize;

                for(int line = y; line < y2; line++) {

                    for(int pixel = x; pixel < x2; pixel++) {

                        Xil_signed16 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_signed16 dest_band[_XILI_CS_MAX_BANDS];

                        CSoperation[i].func_ptr(src_band,
                                                dest_band,
                                                1,
                                                1,  // dummy argument
                                                1); // dummy argument

                        for(b = 0; b < dest_nbands; b++) {
                            *(dest_data[b] +
                              line * dest_scanline_stride[b] +
                              pixel * dest_pixel_stride[b]) =
                                dest_band[b];
                        }

                    } // for pixel
                } // for line
            } // while rl.getNext
        } // general storage implementation
    } // while b1->getNext

    return XIL_SUCCESS;
}
