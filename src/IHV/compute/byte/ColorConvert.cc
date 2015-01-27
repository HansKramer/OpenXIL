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
//  File:	ColorConvert.cc
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:10:58, 03/10/00
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
#pragma ident	"@(#)ColorConvert.cc	1.9\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "XiliCSop.hh"
#include "XiliUtils.hh"

#include "color_convert.hh"

//
// Forward declarations
//
void rgblinear_to_cmy(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void rgblinear_to_cmyk(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void rgblinear_to_ylinear(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);

void rgb709_to_photoycc(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void rgb709_to_ycc601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void rgb709_to_ycc709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void rgb709_to_y601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void rgb709_to_y709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);

void photoycc_to_ycc601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void photoycc_to_ycc709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void photoycc_to_y601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void photoycc_to_y709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);

void ycc601_to_y601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ycc601_to_y709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ycc601_to_rgb709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ycc601_to_photoycc(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ycc601_to_ycc709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);

void ycc709_to_y601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ycc709_to_y709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ycc709_to_rgb709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ycc709_to_photoycc(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ycc709_to_ycc601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);

void cmy_to_rgblinear(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void cmy_to_cmyk(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void cmy_to_ylinear(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void cmy_to_y601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void cmy_to_y709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);

void cmyk_to_rgblinear(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void cmyk_to_cmy(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void cmyk_to_ylinear(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void cmyk_to_y601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void cmyk_to_y709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);

void ylinear_to_rgblinear(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ylinear_to_photoycc(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ylinear_to_ycc601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ylinear_to_ycc709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ylinear_to_cmy(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ylinear_to_cmyk(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ylinear_to_y601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void ylinear_to_y709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);

void y601_to_ycc709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void y601_to_rgb709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void y601_to_photoycc(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void y601_to_ycc601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void y601_to_y709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);

void y709_to_y601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void y709_to_rgb709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void y709_to_photoycc(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void y709_to_ycc601(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
void y709_to_ycc709(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);

//
// Opcode & operation table
//
typedef struct cs_op {
   int   opcode;  
   void  (*func_ptr)(Xil_unsigned8*, Xil_unsigned8*, unsigned int, unsigned int, unsigned int);
} CsOp;

CsOp CSoperation[] = {
	{ XIL_CS_RGBLINEAR_TO_cmyk,     rgblinear_to_cmyk},
	{ XIL_CS_RGBLINEAR_TO_cmy,      rgblinear_to_cmy},
	{ XIL_CS_RGBLINEAR_TO_ylinear,  rgblinear_to_ylinear},
	{ XIL_CS_RGBLINEAR_TO_y709, },
	{ XIL_CS_RGBLINEAR_TO_photoycc, },
	{ XIL_CS_RGBLINEAR_TO_ycc601, },
	{ XIL_CS_RGBLINEAR_TO_ycc709, },
	{ XIL_CS_RGBLINEAR_TO_y601, },
	{ XIL_CS_RGBLINEAR_TO_rgb709, },

	{ XIL_CS_RGB709_TO_photoycc,  rgb709_to_photoycc},
	{ XIL_CS_RGB709_TO_ycc601,    rgb709_to_ycc601},
	{ XIL_CS_RGB709_TO_ycc709,    rgb709_to_ycc709},
	{ XIL_CS_RGB709_TO_y601,      rgb709_to_y601},
	{ XIL_CS_RGB709_TO_y709,      rgb709_to_y709},
	{ XIL_CS_RGB709_TO_rgblinear, },
	{ XIL_CS_RGB709_TO_cmy, },
	{ XIL_CS_RGB709_TO_cmyk, },
	{ XIL_CS_RGB709_TO_ylinear, },

	{ XIL_CS_PHOTOYCC_TO_ycc601,    photoycc_to_ycc601},
	{ XIL_CS_PHOTOYCC_TO_ycc709,    photoycc_to_ycc709},
	{ XIL_CS_PHOTOYCC_TO_y601,      photoycc_to_y601},
	{ XIL_CS_PHOTOYCC_TO_y709,      photoycc_to_y709},
	{ XIL_CS_PHOTOYCC_TO_rgb709, },
	{ XIL_CS_PHOTOYCC_TO_rgblinear, },
	{ XIL_CS_PHOTOYCC_TO_cmy, },
	{ XIL_CS_PHOTOYCC_TO_cmyk, },
	{ XIL_CS_PHOTOYCC_TO_ylinear, },

	{ XIL_CS_YCC601_TO_rgb709,    ycc601_to_rgb709},
	{ XIL_CS_YCC601_TO_photoycc,  ycc601_to_photoycc},
	{ XIL_CS_YCC601_TO_ycc709,    ycc601_to_ycc709},
	{ XIL_CS_YCC601_TO_y601,      ycc601_to_y601},
	{ XIL_CS_YCC601_TO_y709,      ycc601_to_y709},
	{ XIL_CS_YCC601_TO_rgblinear, },
	{ XIL_CS_YCC601_TO_cmy, },
	{ XIL_CS_YCC601_TO_cmyk, },
	{ XIL_CS_YCC601_TO_ylinear, },

	{ XIL_CS_YCC709_TO_rgb709,    ycc709_to_rgb709},
	{ XIL_CS_YCC709_TO_photoycc,  ycc709_to_photoycc},
	{ XIL_CS_YCC709_TO_ycc601,    ycc709_to_ycc601},
	{ XIL_CS_YCC709_TO_y601,      ycc709_to_y601},
	{ XIL_CS_YCC709_TO_y709,      ycc709_to_y709},
	{ XIL_CS_YCC709_TO_rgblinear, },
	{ XIL_CS_YCC709_TO_cmy, },
	{ XIL_CS_YCC709_TO_cmyk, },
	{ XIL_CS_YCC709_TO_ylinear, },

	{ XIL_CS_CMY_TO_rgblinear, cmy_to_rgblinear},
	{ XIL_CS_CMY_TO_cmyk,      cmy_to_cmyk},
	{ XIL_CS_CMY_TO_ylinear,   cmy_to_ylinear},
	{ XIL_CS_CMY_TO_y601,      cmy_to_y601},
	{ XIL_CS_CMY_TO_y709,      cmy_to_y709},
	{ XIL_CS_CMY_TO_photoycc, },
	{ XIL_CS_CMY_TO_ycc601, },
	{ XIL_CS_CMY_TO_ycc709, },
	{ XIL_CS_CMY_TO_rgb709, },

	{ XIL_CS_CMYK_TO_rgblinear, cmyk_to_rgblinear},
	{ XIL_CS_CMYK_TO_cmy,       cmyk_to_cmy},
	{ XIL_CS_CMYK_TO_ylinear,   cmyk_to_ylinear},
	{ XIL_CS_CMYK_TO_y601,      cmyk_to_y601},
	{ XIL_CS_CMYK_TO_y709,      cmyk_to_y709},
	{ XIL_CS_CMYK_TO_photoycc, },
	{ XIL_CS_CMYK_TO_ycc601, },
	{ XIL_CS_CMYK_TO_ycc709, },
	{ XIL_CS_CMYK_TO_rgb709, },

	{ XIL_CS_YLINEAR_TO_rgblinear, ylinear_to_rgblinear},
	{ XIL_CS_YLINEAR_TO_photoycc,  ylinear_to_photoycc},
	{ XIL_CS_YLINEAR_TO_ycc601,    ylinear_to_ycc601},
	{ XIL_CS_YLINEAR_TO_ycc709,    ylinear_to_ycc709},
	{ XIL_CS_YLINEAR_TO_cmy,       ylinear_to_cmy},
	{ XIL_CS_YLINEAR_TO_cmyk,      ylinear_to_cmyk},
	{ XIL_CS_YLINEAR_TO_y601,      ylinear_to_y601},
	{ XIL_CS_YLINEAR_TO_y709,      ylinear_to_y709},
	{ XIL_CS_YLINEAR_TO_rgb709, },

	{ XIL_CS_Y601_TO_rgb709,    y601_to_rgb709},
	{ XIL_CS_Y601_TO_photoycc,  y601_to_photoycc},
	{ XIL_CS_Y601_TO_ycc601,    y601_to_ycc601},
	{ XIL_CS_Y601_TO_ycc709,    y601_to_ycc709},
	{ XIL_CS_Y601_TO_y709,      y601_to_y709},
	{ XIL_CS_Y601_TO_rgblinear, },
	{ XIL_CS_Y601_TO_ylinear, },
	{ XIL_CS_Y601_TO_cmy, },
	{ XIL_CS_Y601_TO_cmyk, },

	{ XIL_CS_Y709_TO_rgb709,    y709_to_rgb709},
	{ XIL_CS_Y709_TO_photoycc,  y709_to_photoycc},
	{ XIL_CS_Y709_TO_ycc601,    y709_to_ycc601},
	{ XIL_CS_Y709_TO_ycc709,    y709_to_ycc709},
	{ XIL_CS_Y709_TO_y601,      y709_to_y601},
	{ XIL_CS_Y709_TO_rgblinear, },
	{ XIL_CS_Y709_TO_ylinear, },
	{ XIL_CS_Y709_TO_cmy, },
	{ XIL_CS_Y709_TO_cmyk, },

	{ 0, NULL}
};


XilStatus
XilDeviceManagerComputeBYTE::ColorConvert(XilOp*       op,
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
      if (opcode == (unsigned int) CSoperation[i].opcode) {
        //
        // Found the appropriate function
        //
        found = 1;
        break;
      }
    }

    //
    // Photycc->Rgb709.
    //
    if (CSoperation[i].opcode == XIL_CS_PHOTOYCC_TO_rgb709) {
      XilStatus status = photoycc_to_rgb709(op, roi, bl);
      return status;
    }

    //
    // Cmy->Photoycc.
    //
    if (CSoperation[i].opcode == XIL_CS_CMY_TO_photoycc) {
      XilStatus status = cmy_to_photoycc(op, roi, bl);
      return status;
    }

    //
    // Cmy->Ycc601.
    //
    if (CSoperation[i].opcode == XIL_CS_CMY_TO_ycc601) {
      XilStatus status = cmy_to_ycc601(op, roi, bl);
      return status;
    }

    //
    // Cmy->Ycc709.
    //
    if (CSoperation[i].opcode == XIL_CS_CMY_TO_ycc709) {
      XilStatus status = cmy_to_ycc709(op, roi, bl);
      return status;
    }

    //
    // Cmy->Rgb709.
    //
    if (CSoperation[i].opcode == XIL_CS_CMY_TO_rgb709) {
      XilStatus status = cmy_to_rgb709(op, roi, bl);
      return status;
    }

    //
    // Cmyk->Photoycc.
    //
    if (CSoperation[i].opcode == XIL_CS_CMYK_TO_photoycc) {
      XilStatus status = cmyk_to_photoycc(op, roi, bl);
      return status;
    }

    //
    // Cmyk->Ycc601.
    //
    if (CSoperation[i].opcode == XIL_CS_CMYK_TO_ycc601) {
      XilStatus status = cmyk_to_ycc601(op, roi, bl);
      return status;
    }

    //
    // Cmyk->Ycc709.
    //
    if (CSoperation[i].opcode == XIL_CS_CMYK_TO_ycc709) {
      XilStatus status = cmyk_to_ycc709(op, roi, bl);
      return status;
    }

    //
    // Cmyk->Rgb709.
    //
    if (CSoperation[i].opcode == XIL_CS_CMYK_TO_rgb709) {
      XilStatus status = cmyk_to_rgb709(op, roi, bl);
      return status;
    }

    //
    // Rgb709->Rgblinear.
    //
    if (CSoperation[i].opcode == XIL_CS_RGB709_TO_rgblinear) {
      XilStatus status = rgb709_to_rgblinear(op, roi, bl);
      return status;
    }

    //
    // Rgb709->Cmy.
    //
    if (CSoperation[i].opcode == XIL_CS_RGB709_TO_cmy) {
      XilStatus status = rgb709_to_cmy(op, roi, bl);
      return status;
    }

    //
    // Rgb709->Cmyk.
    //
    if (CSoperation[i].opcode == XIL_CS_RGB709_TO_cmyk) {
      XilStatus status = rgb709_to_cmyk(op, roi, bl);
      return status;
    }

    //
    // Rgb709->Ylinear.
    //
    if (CSoperation[i].opcode == XIL_CS_RGB709_TO_ylinear) {
      XilStatus status = rgb709_to_ylinear(op, roi, bl);
      return status;
    }

    //
    // Rgblinear->Photoycc.
    //
    if (CSoperation[i].opcode == XIL_CS_RGBLINEAR_TO_photoycc) {
      XilStatus status = rgblinear_to_photoycc(op, roi, bl);
      return status;
    }

    //
    // Rgblinear->Ycc601.
    //
    if (CSoperation[i].opcode == XIL_CS_RGBLINEAR_TO_ycc601) {
      XilStatus status = rgblinear_to_ycc601(op, roi, bl);
      return status;
    }

    //
    // Rgblinear->Ycc709.
    //
    if (CSoperation[i].opcode == XIL_CS_RGBLINEAR_TO_ycc709) {
      XilStatus status = rgblinear_to_ycc709(op, roi, bl);
      return status;
    }

    //
    // Rgblinear->Y601.
    //
    if (CSoperation[i].opcode == XIL_CS_RGBLINEAR_TO_y601) {
      XilStatus status = rgblinear_to_y601(op, roi, bl);
      return status;
    }

    //
    // Rgblinear->Y709.
    //
    if (CSoperation[i].opcode == XIL_CS_RGBLINEAR_TO_y709) {
      XilStatus status = rgblinear_to_y709(op, roi, bl);
      return status;
    }

    //
    // Rgblinear->Rgb709.
    //
    if (CSoperation[i].opcode == XIL_CS_RGBLINEAR_TO_rgb709) {
      XilStatus status = rgblinear_to_rgb709(op, roi, bl);
      return status;
    }

    //
    // Ylinear->Rgb709.
    //
    if (CSoperation[i].opcode == XIL_CS_YLINEAR_TO_rgb709) {
      XilStatus status = ylinear_to_rgb709(op, roi, bl);
      return status;
    }

    //
    // Y601->Rgblinear.
    //
    if (CSoperation[i].opcode == XIL_CS_Y601_TO_rgblinear) {
      XilStatus status = y601_to_rgblinear(op, roi, bl);
      return status;
    }

    //
    // Photoycc->Rgblinear.
    //
    if (CSoperation[i].opcode == XIL_CS_PHOTOYCC_TO_rgblinear) {
      XilStatus status = photoycc_to_rgblinear(op, roi, bl);
      return status;
    }

    //
    // Photoycc->Cmy.
    //
    if (CSoperation[i].opcode == XIL_CS_PHOTOYCC_TO_cmy) {
      XilStatus status = photoycc_to_cmy(op, roi, bl);
      return status;
    }

    //
    // Photoycc->Cmyk.
    //
    if (CSoperation[i].opcode == XIL_CS_PHOTOYCC_TO_cmyk) {
      XilStatus status = photoycc_to_cmyk(op, roi, bl);
      return status;
    }

    //
    // Photoycc->Ylinear.
    //
    if (CSoperation[i].opcode == XIL_CS_PHOTOYCC_TO_ylinear) {
      XilStatus status = photoycc_to_ylinear(op, roi, bl);
      return status;
    }

    //
    // Y601->Ylinear.
    //
    if (CSoperation[i].opcode == XIL_CS_Y601_TO_ylinear) {
      XilStatus status = y601_to_ylinear(op, roi, bl);
      return status;
    }

    //
    // Y601->Cmy.
    //
    if (CSoperation[i].opcode == XIL_CS_Y601_TO_cmy) {
      XilStatus status = y601_to_cmy(op, roi, bl);
      return status;
    }

    //
    // Y601->Cmyk.
    //
    if (CSoperation[i].opcode == XIL_CS_Y601_TO_cmyk) {
      XilStatus status = y601_to_cmyk(op, roi, bl);
      return status;
    }

    //
    // Y709->Rgblinear.
    //
    if (CSoperation[i].opcode == XIL_CS_Y709_TO_rgblinear) {
      XilStatus status = y709_to_rgblinear(op, roi, bl);
      return status;
    }

    //
    // Y709->Cmy.
    //
    if (CSoperation[i].opcode == XIL_CS_Y709_TO_cmy) {
      XilStatus status = y709_to_cmy(op, roi, bl);
      return status;
    }

    //
    // Y709->Cmyk.
    //
    if (CSoperation[i].opcode == XIL_CS_Y709_TO_cmyk) {
      XilStatus status = y709_to_cmyk(op, roi, bl);
      return status;
    }

    //
    // Y709->Ylinear.
    //
    if (CSoperation[i].opcode == XIL_CS_Y709_TO_ylinear) {
      XilStatus status = y709_to_ylinear(op, roi, bl);
      return status;
    }

    //
    // YCC601->Rgblinear.
    //
    if (CSoperation[i].opcode == XIL_CS_YCC601_TO_rgblinear) {
      XilStatus status = ycc601_to_rgblinear(op, roi, bl);
      return status;
    }

    //
    // YCC601->Cmy.
    //
    if (CSoperation[i].opcode == XIL_CS_YCC601_TO_cmy) {
      XilStatus status = ycc601_to_cmy(op, roi, bl);
      return status;
    }

    //
    // YCC601->Cmyk.
    //
    if (CSoperation[i].opcode == XIL_CS_YCC601_TO_cmyk) {
      XilStatus status = ycc601_to_cmyk(op, roi, bl);
      return status;
    }

    //
    // YCC601->Ylinear.
    //
    if (CSoperation[i].opcode == XIL_CS_YCC601_TO_ylinear) {
      XilStatus status = ycc601_to_ylinear(op, roi, bl);
      return status;
    }

    //
    // YCC709->Rgblinear.
    //
    if (CSoperation[i].opcode == XIL_CS_YCC709_TO_rgblinear) {
      XilStatus status = ycc709_to_rgblinear(op, roi, bl);
      return status;
    }

    //
    // YCC709->Cmy.
    //
    if (CSoperation[i].opcode == XIL_CS_YCC709_TO_cmy) {
      XilStatus status = ycc709_to_cmy(op, roi, bl);
      return status;
    }

    //
    // YCC709->Cmyk.
    //
    if (CSoperation[i].opcode == XIL_CS_YCC709_TO_cmyk) {
      XilStatus status = ycc709_to_cmyk(op, roi, bl);
      return status;
    }

    //
    // YCC709->Ylinear.
    //
    if (CSoperation[i].opcode == XIL_CS_YCC709_TO_ylinear) {
      XilStatus status = ycc709_to_ylinear(op, roi, bl);
      return status;
    }

    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

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

//
// Note : This is the only routine which is a member of the
//        XilDeviceManagerComputeBYTE class as it needs to create
//        the array's dynamically.
//
XilStatus
XilDeviceManagerComputeBYTE::photoycc_to_rgb709(XilOp* op,
                                                XilRoi* roi,
                                                XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

    //
    //  Get the system state for reporting errors.
    //
    XilSystemState* state = dest->getSystemState();

    //
    // Color-convert PhotoYCC to RGB709
    //
    Xil_signed32* PhotoY = getPhotoYArray(state);
    if(PhotoY == NULL) {
        return XIL_FAILURE;
    } 

    Xil_signed32* PhotoCb = getPhotoCbArray(state);
    if(PhotoCb == NULL) {
        return XIL_FAILURE;
    } 

    Xil_signed32* PhotoYCb = getPhotoYCbArray(state);
    if(PhotoYCb == NULL) {
        return XIL_FAILURE;
    } 

    Xil_signed32* PhotoYCr = getPhotoYCrArray(state);
    if(PhotoYCr == NULL) {
        return XIL_FAILURE;
    } 

    Xil_signed32* PhotoCr = getPhotoCrArray(state);
    if(PhotoCr == NULL) {
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

        //
        //  Test to see if all of our storage is of type XIL_PIXEL_SEQUENTIAL.
        //  If so, implement an loop optimized for pixel-sequential storage.
        //
        if((src_storage.isType(XIL_PIXEL_SEQUENTIAL)) &&
           (dest_storage.isType(XIL_PIXEL_SEQUENTIAL))) {

            unsigned int   src_pixel_stride;
            unsigned int   src_scanline_stride;
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                       &src_scanline_stride,
                                       NULL, NULL,
                                       (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                    (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                    (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                  Xil_unsigned8* src_pixel = src_scanline;
                  Xil_unsigned8* dest_pixel = dest_scanline;

                  for (unsigned int i = 0; i < xsize; i++) {
                     Xil_signed32 y1, red, green, blue;                         
                                    
                     y1 = PhotoY[*src_pixel];
                     blue = (y1 + PhotoCb[*(src_pixel+1)]) >> 12;
                     green = (y1 +
                              PhotoYCb[*(src_pixel+1)] +
                              PhotoYCr[*(src_pixel+2)] ) >> 12; 
                     red = (y1 + PhotoCr[*(src_pixel+2)]) >> 12;
                                    
                     if (blue < XIL_MINBYTE)                           
                       *dest_pixel = XIL_MINBYTE;                       
                     else if (blue > XIL_MAXBYTE)                      
                       *dest_pixel = XIL_MAXBYTE;                       
                     else                                
                       *dest_pixel = (Xil_unsigned8)blue;                 
                                    
                     if (green < XIL_MINBYTE)                           
                       *(dest_pixel+1) = XIL_MINBYTE;                       
                     else if (green > XIL_MAXBYTE)                      
                       *(dest_pixel+1) = XIL_MAXBYTE;                       
                     else                                
                       *(dest_pixel+1) = (Xil_unsigned8)green;                 
                                    
                     if (red < XIL_MINBYTE)                           
                       *(dest_pixel+2) = XIL_MINBYTE;                       
                     else if (red > XIL_MAXBYTE)                      
                       *(dest_pixel+2) = XIL_MAXBYTE;                       
                     else                                
                       *(dest_pixel+2) = (Xil_unsigned8)red;                 

                     src_pixel += src_pixel_stride;
                     dest_pixel += dest_pixel_stride;
                  }

                  //
                  // Move to next scanline
                  //
                  src_scanline += src_scanline_stride;
                  dest_scanline += dest_scanline_stride;

                } while (--ysize);
            }

        } else { // pixel_type
            //
            // General storage implementation
            //
            int b; // band index variable

            //
            // Set up band-indexed stride and data pointers.
            //
            Xil_unsigned8* src_data[3];
            unsigned int src_pixel_stride[3];
            unsigned int src_scanline_stride[3];

            for(b = 0; b < 3; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[3];
            unsigned int dest_pixel_stride[3];
            unsigned int dest_scanline_stride[3];

            for(b = 0; b < 3; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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
                        //
                        // Init an array of pointers to the 3 src band values
                        //
                        Xil_unsigned8* src_pixel[3];
                        for(b = 0; b < 3; b++) {
                            src_pixel[b] = src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b];
                        }

                        //
                        // Convert to the dest color space
                        //
                        Xil_signed32 y1 = PhotoY[*src_pixel[0]];
                        Xil_signed32 blue = (y1 + PhotoCb[*src_pixel[1]]) >> 12;
                        Xil_signed32 green = (y1 +
                                 PhotoYCb[*src_pixel[1]] +
                                 PhotoYCr[*src_pixel[2]] ) >> 12;
                        Xil_signed32 red = (y1 + PhotoCr[*src_pixel[2]]) >> 12;

                        //
                        // Init an array of pointers to the 3 dest band values
                        //
                        Xil_unsigned8* dest_pixel[3];
                        for(b = 0; b < 3; b++) {
                            dest_pixel[b] = dest_data[b] +
                                line * dest_scanline_stride[b] +
                                pixel * dest_pixel_stride[b];
                        }

                        //
                        // Assign the clamped result to the dest addresses
                        //
                        *dest_pixel[0] = _XILI_CLAMP_U8(blue);
                        *dest_pixel[1] = _XILI_CLAMP_U8(green);
                        *dest_pixel[2] = _XILI_CLAMP_U8(red);

                    } // for pixel
                } // for line
            } // while rl.getNext
        } // general storage implementation

    } // while b1->getNext

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeBYTE::cmy_to_photoycc(XilOp*       op,
					                      XilRoi*      roi,
					                      XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float r7, g7, b7, photoy, cb, cr;
                      Xil_unsigned8 red, green, blue;

                      _XILI_CMY_TO_L_B(*(src_pixel),
                                       *(src_pixel+1),
                                       *(src_pixel+2),
                                       &red,
                                       &green,
                                       &blue);

                      r7 = _XILI_NORMALIZE_B(_L2NLbtable[red]);
                      g7 = _XILI_NORMALIZE_B(_L2NLbtable[green]);
                      b7 = _XILI_NORMALIZE_B(_L2NLbtable[blue]);

                      _XILI_NL_TO_YCC601(r7, g7, b7, &photoy, &cb, &cr);

                      _XILI_QUANTIZE_PHOTO_B(photoy,
                                             cb,
                                             cr,
                                             dest_pixel,
                                             dest_pixel+1,
                                             dest_pixel+2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float r7, g7, b7, photoy, cb, cr;
                          Xil_unsigned8 red, green, blue;

                          _XILI_CMY_TO_L_B(*(src_band),
                                       *(src_band+1),
                                       *(src_band+2),
                                       &red,
                                       &green,
                                       &blue);

                          r7 = _XILI_NORMALIZE_B(_L2NLbtable[red]);
                          g7 = _XILI_NORMALIZE_B(_L2NLbtable[green]);
                          b7 = _XILI_NORMALIZE_B(_L2NLbtable[blue]);

                          _XILI_NL_TO_YCC601(r7, g7, b7, &photoy, &cb, &cr);

                          _XILI_QUANTIZE_PHOTO_B(photoy,
                                             cb,
                                             cr,
                                             dest_band,
                                             dest_band+1,
                                             dest_band+2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::cmy_to_ycc601(XilOp*       op,
					                      XilRoi*      roi,
					                      XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float r7, g7, b7, y601, cb, cr;
                      Xil_unsigned8 red, green, blue;

                      _XILI_CMY_TO_L_B(*(src_pixel),
                                       *(src_pixel+1),
                                       *(src_pixel+2),
                                       &red,
                                       &green,
                                       &blue);

                      r7 = _XILI_NORMALIZE_B(_L2NLbtable[red]);
                      g7 = _XILI_NORMALIZE_B(_L2NLbtable[green]);
                      b7 = _XILI_NORMALIZE_B(_L2NLbtable[blue]);

                      _XILI_NL_TO_YCC601(r7, g7, b7, &y601, &cb, &cr);

                      _XILI_QUANTIZE_YCC601_B(y601,
                                             cb,
                                             cr,
                                             dest_pixel,
                                             dest_pixel+1,
                                             dest_pixel+2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float r7, g7, b7, y601, cb, cr;
                          Xil_unsigned8 red, green, blue;

                          _XILI_CMY_TO_L_B(*(src_band),
                                       *(src_band+1),
                                       *(src_band+2),
                                       &red,
                                       &green,
                                       &blue);


                          r7 = _XILI_NORMALIZE_B(_L2NLbtable[red]);
                          g7 = _XILI_NORMALIZE_B(_L2NLbtable[green]);
                          b7 = _XILI_NORMALIZE_B(_L2NLbtable[blue]);

                          _XILI_NL_TO_YCC601(r7, g7, b7, &y601, &cb, &cr);

                          _XILI_QUANTIZE_YCC601_B(y601,
                                             cb,
                                             cr,
                                             dest_band,
                                             dest_band+1,
                                             dest_band+2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::cmy_to_ycc709(XilOp*       op,
					                      XilRoi*      roi,
					                      XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float r7, g7, b7, y709, cb, cr;
                      Xil_unsigned8 red, green, blue;

                      _XILI_CMY_TO_L_B(*(src_pixel),
                                       *(src_pixel+1),
                                       *(src_pixel+2),
                                       &red,
                                       &green,
                                       &blue);

                      r7 = _XILI_NORMALIZE_B(_L2NLbtable[red]);
                      g7 = _XILI_NORMALIZE_B(_L2NLbtable[green]);
                      b7 = _XILI_NORMALIZE_B(_L2NLbtable[blue]);

                      _XILI_NL_TO_YCC709(r7, g7, b7, &y709, &cb, &cr);

                      _XILI_QUANTIZE_YCC709_B(y709,
                                             cb,
                                             cr,
                                             dest_pixel,
                                             dest_pixel+1,
                                             dest_pixel+2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float r7, g7, b7, y709, cb, cr;
                          Xil_unsigned8 red, green, blue;

                          _XILI_CMY_TO_L_B(*(src_band),
                                       *(src_band+1),
                                       *(src_band+2),
                                       &red,
                                       &green,
                                       &blue);

                          r7 = _XILI_NORMALIZE_B(_L2NLbtable[red]);
                          g7 = _XILI_NORMALIZE_B(_L2NLbtable[green]);
                          b7 = _XILI_NORMALIZE_B(_L2NLbtable[blue]);

                          _XILI_NL_TO_YCC709(r7, g7, b7, &y709, &cb, &cr);

                          _XILI_QUANTIZE_YCC709_B(y709,
                                             cb,
                                             cr,
                                             dest_band,
                                             dest_band+1,
                                             dest_band+2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::cmy_to_rgb709(XilOp*       op,
					                      XilRoi*      roi,
					                      XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      Xil_unsigned8 red, green, blue;

                      _XILI_CMY_TO_L_B(*(src_pixel),
                                       *(src_pixel+1),
                                       *(src_pixel+2),
                                       &red,
                                       &green,
                                       &blue);

                      *dest_pixel = _L2NLbtable[blue];
                      *(dest_pixel+1) = _L2NLbtable[green];
                      *(dest_pixel+2) = _L2NLbtable[red];

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          Xil_unsigned8 red, green, blue;

                          _XILI_CMY_TO_L_B(*(src_band),
                                       *(src_band+1),
                                       *(src_band+2),
                                       &red,
                                       &green,
                                       &blue);


                          *dest_band = _L2NLbtable[blue];
                          *(dest_band+1) = _L2NLbtable[green];
                          *(dest_band+2) = _L2NLbtable[red];
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::cmyk_to_photoycc(XilOp*       op,
					                      XilRoi*      roi,
					                      XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float r7, g7, b7, photoy, cb, cr;
                      Xil_unsigned8 red, green, blue;

                      _XILI_CMYK_TO_L_B(*(src_pixel),
                                       *(src_pixel+1),
                                       *(src_pixel+2),
                                       *(src_pixel+3),
                                       &red,
                                       &green,
                                       &blue);

                      r7 = _XILI_NORMALIZE_B(_L2NLbtable[red]);
                      g7 = _XILI_NORMALIZE_B(_L2NLbtable[green]);
                      b7 = _XILI_NORMALIZE_B(_L2NLbtable[blue]);

                      _XILI_NL_TO_YCC601(r7, g7, b7, &photoy, &cb, &cr);

                      _XILI_QUANTIZE_PHOTO_B(photoy, 
                                             cb, 
                                             cr, 
                                             dest_pixel, 
                                             dest_pixel+1, 
                                             dest_pixel+2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float r7, g7, b7, photoy, cb, cr;
                          Xil_unsigned8 red, green, blue;

                          _XILI_CMYK_TO_L_B(*(src_band),
                                           *(src_band+1),
                                           *(src_band+2),
                                           *(src_band+3),
                                           &red,
                                           &green,
                                           &blue);

                          r7 = _XILI_NORMALIZE_B(_L2NLbtable[red]);
                          g7 = _XILI_NORMALIZE_B(_L2NLbtable[green]);
                          b7 = _XILI_NORMALIZE_B(_L2NLbtable[blue]);

                          _XILI_NL_TO_YCC601(r7, g7, b7, &photoy, &cb, &cr);

                          _XILI_QUANTIZE_PHOTO_B(photoy, 
                                                 cb, 
                                                 cr, 
                                                 dest_band, 
                                                 dest_band+1, 
                                                 dest_band+2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::cmyk_to_ycc601(XilOp*       op,
					                      XilRoi*      roi,
					                      XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float r7, g7, b7, y601, cb, cr;
                      Xil_unsigned8 r6, g6, b6;

                      _XILI_CMYK_TO_L_B(*(src_pixel),
                                       *(src_pixel+1),
                                       *(src_pixel+2),
                                       *(src_pixel+3),
                                       &r6,
                                       &g6,
                                       &b6);

                      r7 = _XILI_NORMALIZE_B(_L2NLbtable[r6]);
                      g7 = _XILI_NORMALIZE_B(_L2NLbtable[g6]);
                      b7 = _XILI_NORMALIZE_B(_L2NLbtable[b6]);

                      _XILI_NL_TO_YCC601(r7, g7, b7, &y601, &cb, &cr);

                      _XILI_QUANTIZE_YCC601_B(y601, 
                                             cb, 
                                             cr, 
                                             dest_pixel, 
                                             dest_pixel+1, 
                                             dest_pixel+2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float r7, g7, b7, y601, cb, cr;
                          Xil_unsigned8 r6, g6, b6;

                          _XILI_CMYK_TO_L_B(*(src_band),
                                           *(src_band+1),
                                           *(src_band+2),
                                           *(src_band+3),
                                           &r6,
                                           &g6,
                                           &b6);

                          r7 = _XILI_NORMALIZE_B(_L2NLbtable[r6]);
                          g7 = _XILI_NORMALIZE_B(_L2NLbtable[g6]);
                          b7 = _XILI_NORMALIZE_B(_L2NLbtable[b6]);

                          _XILI_NL_TO_YCC601(r7, g7, b7, &y601, &cb, &cr);

                          _XILI_QUANTIZE_YCC601_B(y601, 
                                                 cb, 
                                                 cr, 
                                                 dest_band, 
                                                 dest_band+1, 
                                                 dest_band+2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::cmyk_to_ycc709(XilOp*       op,
					                      XilRoi*      roi,
					                      XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float r7, g7, b7, y709, cb, cr;
                      Xil_unsigned8 r6, g6, b6;

                      _XILI_CMYK_TO_L_B(*(src_pixel),
                                       *(src_pixel+1),
                                       *(src_pixel+2),
                                       *(src_pixel+3),
                                       &r6,
                                       &g6,
                                       &b6);

                      r7 = _XILI_NORMALIZE_B(_L2NLbtable[r6]);
                      g7 = _XILI_NORMALIZE_B(_L2NLbtable[g6]);
                      b7 = _XILI_NORMALIZE_B(_L2NLbtable[b6]);

                      _XILI_NL_TO_YCC709(r7, g7, b7, &y709, &cb, &cr);

                      _XILI_QUANTIZE_YCC709_B(y709, 
                                             cb, 
                                             cr, 
                                             dest_pixel, 
                                             dest_pixel+1, 
                                             dest_pixel+2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float r7, g7, b7, y709, cb, cr;
                          Xil_unsigned8 r6, g6, b6;

                          _XILI_CMYK_TO_L_B(*(src_band),
                                           *(src_band+1),
                                           *(src_band+2),
                                           *(src_band+3),
                                           &r6,
                                           &g6,
                                           &b6);

                          r7 = _XILI_NORMALIZE_B(_L2NLbtable[r6]);
                          g7 = _XILI_NORMALIZE_B(_L2NLbtable[g6]);
                          b7 = _XILI_NORMALIZE_B(_L2NLbtable[b6]);

                          _XILI_NL_TO_YCC709(r7, g7, b7, &y709, &cb, &cr);

                          _XILI_QUANTIZE_YCC709_B(y709, 
                                                 cb, 
                                                 cr, 
                                                 dest_band, 
                                                 dest_band+1, 
                                                 dest_band+2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::cmyk_to_rgb709(XilOp*       op,
					                      XilRoi*      roi,
					                      XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      Xil_unsigned8 red, green, blue;

                      _XILI_CMYK_TO_L_B(*(src_pixel),
                                       *(src_pixel+1),
                                       *(src_pixel+2),
                                       *(src_pixel+3),
                                       &red,
                                       &green,
                                       &blue);

                      *dest_pixel = _L2NLbtable[blue];
                      *(dest_pixel + 1) = _L2NLbtable[green];
                      *(dest_pixel + 2) = _L2NLbtable[red];

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          Xil_unsigned8 red, green, blue;

                          _XILI_CMYK_TO_L_B(*(src_band),
                                           *(src_band+1),
                                           *(src_band+2),
                                           *(src_band+3),
                                           &red,
                                           &green,
                                           &blue);

                          *dest_band = _L2NLbtable[blue];
                          *(dest_band + 1) = _L2NLbtable[green];
                          *(dest_band + 2) = _L2NLbtable[red];
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::rgb709_to_rgblinear(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      *dest_pixel = _NL2Lbtable[*(src_pixel)];
                      *(dest_pixel + 1) = _NL2Lbtable[*(src_pixel + 1)];
                      *(dest_pixel + 2) = _NL2Lbtable[*(src_pixel + 2)];

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          *dest_band = _NL2Lbtable[*(src_band)];
                          *(dest_band + 1) = _NL2Lbtable[*(src_band + 1)];
                          *(dest_band + 2) = _NL2Lbtable[*(src_band + 2)];
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::rgb709_to_cmy(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      *dest_pixel = 255 - _NL2Lbtable[*(src_pixel + 2)];
                      *(dest_pixel + 1) = 255 - _NL2Lbtable[*(src_pixel + 1)];
                      *(dest_pixel + 2) = 255 - _NL2Lbtable[*(src_pixel)];

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          *dest_band = 255 - _NL2Lbtable[*(src_band + 2)];
                          *(dest_band + 1) = 255 - _NL2Lbtable[*(src_band + 1)];
                          *(dest_band + 2) = 255 - _NL2Lbtable[*(src_band)];
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::rgb709_to_cmyk(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      *dest_pixel = 255 - _NL2Lbtable[*(src_pixel + 2)];
                      *(dest_pixel + 1) = 255 - _NL2Lbtable[*(src_pixel + 1)];
                      *(dest_pixel + 2) = 255 - _NL2Lbtable[*(src_pixel)];
                      *(dest_pixel + 3) = XIL_MINBYTE;

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          *dest_band = 255 - _NL2Lbtable[*(src_band + 2)];
                          *(dest_band + 1) = 255 - _NL2Lbtable[*(src_band + 1)];
                          *(dest_band + 2) = 255 - _NL2Lbtable[*(src_band)];
                          *(dest_band + 3) = XIL_MINBYTE;
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::rgb709_to_ylinear(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      double tmp;

                      tmp = _XILI_L_TO_Ylinear(_NL2Lbtable[*(src_pixel + 2)],
                                             _NL2Lbtable[*(src_pixel + 1)],
                                             _NL2Lbtable[*(src_pixel)]);

                      *dest_pixel = _XILI_ROUND_U8((float)tmp);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          double tmp;

                          tmp = _XILI_L_TO_Ylinear(_NL2Lbtable[*(src_band + 2)],
                                                 _NL2Lbtable[*(src_band + 1)],
                                                 _NL2Lbtable[*(src_band)]);

                          *dest_band = _XILI_ROUND_U8((float)y);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::rgblinear_to_y709(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      double tmp;

                      tmp = _XILI_NL_TO_Y709(_L2NLbtable[*(src_pixel + 2)],
                                           _L2NLbtable[*(src_pixel + 1)],
                                           _L2NLbtable[*(src_pixel)]);

                      *dest_pixel = _XILI_QUANTIZE_Y709((float)tmp);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          double tmp;

                          tmp = _XILI_NL_TO_Y709(_L2NLbtable[*(src_band + 2)],
                                               _L2NLbtable[*(src_band + 1)],
                                               _L2NLbtable[*(src_band)]);

                          *dest_band = _XILI_QUANTIZE_Y709((float)y);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::rgblinear_to_photoycc(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float red, green, blue, y601, cb, cr;

                      blue  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_pixel)]);
                      green  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_pixel + 1)]);
                      red  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_pixel + 2)]);

                      _XILI_NL_TO_YCC601(red, green, blue, &y601, &cb, &cr);

                      _XILI_QUANTIZE_PHOTO_B(y601,
                                             cb,
                                             cr,
                                             dest_pixel,
                                             dest_pixel + 1,
                                             dest_pixel + 2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float red, green, blue, y601, cb, cr;

                          blue  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_band)]);
                          green  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_band + 1)]);
                          red  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_band + 2)]);

                          _XILI_NL_TO_YCC601(red, green, blue, &y601, &cb, &cr);

                          _XILI_QUANTIZE_PHOTO_B(y601,
                                                 cb,
                                                 cr,
                                                 dest_band,
                                                 dest_band + 1,
                                                 dest_band + 2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::rgblinear_to_ycc601(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float red, green, blue, y601, cb, cr;

                      blue  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_pixel)]);
                      green  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_pixel + 1)]);
                      red  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_pixel + 2)]);

                      _XILI_NL_TO_YCC601(red, green, blue, &y601, &cb, &cr);

                      _XILI_QUANTIZE_YCC601_B(y601,
                                              cb,
                                              cr,
                                              dest_pixel,
                                              dest_pixel + 1,
                                              dest_pixel + 2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float red, green, blue, y601, cb, cr;

                          blue  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_band)]);
                          green  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_band + 1)]);
                          red  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_band + 2)]);

                          _XILI_NL_TO_YCC601(red, green, blue, &y601, &cb, &cr);

                          _XILI_QUANTIZE_YCC601_B(y601,
                                                  cb,
                                                  cr,
                                                  dest_band,
                                                  dest_band + 1,
                                                  dest_band + 2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::rgblinear_to_ycc709(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float red, green, blue, y709, cb, cr;

                      blue  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_pixel)]);
                      green  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_pixel + 1)]);
                      red  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_pixel + 2)]);

                      _XILI_NL_TO_YCC709(red, green, blue, &y709, &cb, &cr);

                      _XILI_QUANTIZE_YCC709_B(y709,
                                              cb,
                                              cr,
                                              dest_pixel,
                                              dest_pixel + 1,
                                              dest_pixel + 2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float red, green, blue, y709, cb, cr;

                          blue  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_band)]);
                          green  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_band + 1)]);
                          red  = _XILI_NORMALIZE_B(_L2NLbtable[*(src_band + 2)]);

                          _XILI_NL_TO_YCC709(red, green, blue, &y709, &cb, &cr);

                          _XILI_QUANTIZE_YCC709_B(y709,
                                                  cb,
                                                  cr,
                                                  dest_band,
                                                  dest_band + 1,
                                                  dest_band + 2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::rgblinear_to_y601(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      double tmp;

                      tmp  = _XILI_NL_TO_Y601(_L2NLbtable[*(src_pixel + 2)],
                                            _L2NLbtable[*(src_pixel + 1)],
                                            _L2NLbtable[*(src_pixel)]);

                      *dest_pixel = _XILI_QUANTIZE_Y601((float)tmp);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          double tmp;

                          tmp  = _XILI_NL_TO_Y601(_L2NLbtable[*(src_band + 2)],
                                                _L2NLbtable[*(src_band + 1)],
                                                _L2NLbtable[*(src_band)]);

                          *dest_band = _XILI_QUANTIZE_Y601((float)tmp);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::rgblinear_to_rgb709(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      *dest_pixel  = _L2NLbtable[*(src_pixel)];
                      *(dest_pixel + 1)  = _L2NLbtable[*(src_pixel + 1)];
                      *(dest_pixel + 2)  = _L2NLbtable[*(src_pixel + 2)];

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          *dest_band  = _L2NLbtable[*(src_band)];
                          *(dest_band + 1)  = _L2NLbtable[*(src_band + 1)];
                          *(dest_band + 2)  = _L2NLbtable[*(src_band + 2)];
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::ylinear_to_rgb709(XilOp*       op,
					                      XilRoi*      roi,
					                      XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      *dest_pixel =
                      *(dest_pixel + 1) =
                      *(dest_pixel + 2) = _L2NLbtable[*(src_pixel)];

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          *dest_band =
                          *(dest_band + 1) =
                          *(dest_band + 2) = _L2NLbtable[*(src_band)];
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::photoycc_to_rgblinear(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y601, cb, cr, red, green, blue;

                      _XILI_NORMALIZE_PHOTO_B(*(src_pixel),
                                              *(src_pixel+1),
                                              *(src_pixel+2),
                                              &y601, &cb, &cr);

                      _XILI_YCC601_TO_NL(y601, cb, cr, &red, &green, &blue);

                      if(red >= 0.081F) {
                        int index;

                        index = (*(int *)(&(red)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *(dest_pixel + 2) = _NL2Ltable[index - 3371];
                      } else {
                        *(dest_pixel + 2) = _XILI_ROUND_U8(red * 56.61F);
                      }

                      if(green >= 0.081F) {
                        int index;

                        index = (*(int *)(&(green)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *(dest_pixel + 1) = _NL2Ltable[index - 3371];
                      } else {
                        *(dest_pixel + 1) = _XILI_ROUND_U8(green * 56.61F);
                      }

                      if(blue >= 0.081F) {
                        int index;

                        index = (*(int *)(&(blue)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *(dest_pixel) = _NL2Ltable[index - 3371];
                      } else {
                        *(dest_pixel) = _XILI_ROUND_U8(blue * 56.61F);
                      }

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                         float y601, cb, cr, red, green, blue;

                         _XILI_NORMALIZE_PHOTO_B(*(src_band),
                                                 *(src_band+1),
                                                 *(src_band+2),
                                                 &y601, &cb, &cr);
   
                         _XILI_YCC601_TO_NL(y601, cb, cr, &red, &green, &blue);
   
                         if(red >= 0.081F) {
                           int index;
                           index = (*(int *)(&(red)) & FMASK) >> 13;
   
                           if(index < 3371) {
                             index = 3371;
                           }
   
                           if(index > 7176) {
                             index = 7176;
                           }
   
                           *(dest_band + 2) = _NL2Ltable[index - 3371];
                         } else {
                           *(dest_band + 2) = _XILI_ROUND_U8(red * 56.61F);
                         }
   
                         if(green >= 0.081F) {
                           int index;
   
                           index = (*(int *)(&(green)) & FMASK) >> 13;
   
                           if(index < 3371) {
                             index = 3371;
                           }
   
                           if(index > 7176) {
                             index = 7176;
                           }
   
                           *(dest_band + 1) = _NL2Ltable[index - 3371];
                         } else {
                           *(dest_band + 1) = _XILI_ROUND_U8(green * 56.61F);
                         }
   
                         if(blue >= 0.081F) {
                           int index;
   
                           index = (*(int *)(&(blue)) & FMASK) >> 13;
   
                           if(index < 3371) {
                             index = 3371;
                           }
   
                           if(index > 7176) {
                             index = 7176;
                           }
   
                           *(dest_band) = _NL2Ltable[index - 3371];
                         } else {
                           *(dest_band) = _XILI_ROUND_U8(blue * 56.61F);
                         }
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::photoycc_to_cmy(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y6, cb6, cr6, r7, g7, b7;
                      Xil_unsigned8 r6, g6, b6;

                      _XILI_NORMALIZE_PHOTO_B(*(src_pixel),
                                              *(src_pixel+1),
                                              *(src_pixel+2),
                                              &y6, &cb6, &cr6);

                      _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);

                      if(r7 >= 0.081F) {
                        int index;
                        index = (*(int *)(&(r7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r6 = _NL2Ltable[index - 3371];
                      } else {
                        r6 = _XILI_ROUND_U8(r7 * 56.61F);
                      }

                      if(g7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(g7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        g6 = _NL2Ltable[index - 3371];
                      } else {
                        g6 = _XILI_ROUND_U8(g7 * 56.61F);
                      }

                      if(b7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(b7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        b6 = _NL2Ltable[index - 3371];
                      } else {
                        b6 = _XILI_ROUND_U8(b7 * 56.61F);
                      }

                      _XILI_L_TO_CMY_B(r6, g6, b6,
                                       dest_pixel,
                                       dest_pixel + 1,
                                       dest_pixel + 2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y6, cb6, cr6, r7, g7, b7;
                          Xil_unsigned8 r6, g6, b6;

                          _XILI_NORMALIZE_PHOTO_B(*(src_band),
                                                  *(src_band+1),
                                                  *(src_band+2),
                                                  &y6, &cb6, &cr6);

                          _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);

                          if(r7 >= 0.081F) {
                            int index;
                            index = (*(int *)(&(r7)) & FMASK) >> 13;

                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            r6 = _NL2Ltable[index - 3371];
                          } else {
                            r6 = _XILI_ROUND_U8(r7 * 56.61F);
                          }
    
                          if(g7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(g7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            g6 = _NL2Ltable[index - 3371];
                          } else {
                            g6 = _XILI_ROUND_U8(g7 * 56.61F);
                          }
    
                          if(b7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(b7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            b6 = _NL2Ltable[index - 3371];
                          } else {
                            b6 = _XILI_ROUND_U8(b7 * 56.61F);
                          }

                          _XILI_L_TO_CMY_B(r6, g6, b6,
                                           dest_band,
                                           dest_band + 1,
                                           dest_band + 2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::photoycc_to_cmyk(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y6, cb6, cr6, r7, g7, b7;
                      Xil_unsigned8 r6, g6, b6;

                      _XILI_NORMALIZE_PHOTO_B(*(src_pixel),
                                              *(src_pixel+1),
                                              *(src_pixel+2),
                                              &y6, &cb6, &cr6);

                      _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);

                      if(r7 >= 0.081F) {
                        int index;
                        index = (*(int *)(&(r7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r6 = _NL2Ltable[index - 3371];
                      } else {
                        r6 = _XILI_ROUND_U8(r7 * 56.61F);
                      }

                      if(g7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(g7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        g6 = _NL2Ltable[index - 3371];
                      } else {
                        g6 = _XILI_ROUND_U8(g7 * 56.61F);
                      }

                      if(b7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(b7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        b6 = _NL2Ltable[index - 3371];
                      } else {
                        b6 = _XILI_ROUND_U8(b7 * 56.61F);
                      }

                      _XILI_L_TO_CMY_B(r6, g6, b6,
                                       dest_pixel,
                                       dest_pixel + 1,
                                       dest_pixel + 2);
                      *(dest_pixel + 3) = XIL_MINBYTE;

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y6, cb6, cr6, r7, g7, b7;
                          Xil_unsigned8 r6, g6, b6;

                          _XILI_NORMALIZE_PHOTO_B(*(src_band),
                                                  *(src_band+1),
                                                  *(src_band+2),
                                                  &y6, &cb6, &cr6);

                          _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);

                          if(r7 >= 0.081F) {
                            int index;
                            index = (*(int *)(&(r7)) & FMASK) >> 13;

                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            r6 = _NL2Ltable[index - 3371];
                          } else {
                            r6 = _XILI_ROUND_U8(r7 * 56.61F);
                          }
    
                          if(g7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(g7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            g6 = _NL2Ltable[index - 3371];
                          } else {
                            g6 = _XILI_ROUND_U8(g7 * 56.61F);
                          }
    
                          if(b7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(b7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            b6 = _NL2Ltable[index - 3371];
                          } else {
                            b6 = _XILI_ROUND_U8(b7 * 56.61F);
                          }

                          _XILI_L_TO_CMY_B(r6, g6, b6,
                                           dest_band,
                                           dest_band + 1,
                                           dest_band + 2);
                          *(dest_band + 3) = XIL_MINBYTE;
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::photoycc_to_ylinear(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y6, cb6, cr6, r7, g7, b7;
                      double ylinear;
                      Xil_unsigned8 r6, g6, b6;

                      _XILI_NORMALIZE_PHOTO_B(*(src_pixel),
                                              *(src_pixel+1),
                                              *(src_pixel+2),
                                              &y6, &cb6, &cr6);

                      _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);

                      if(r7 >= 0.081F) {
                        int index;
                        index = (*(int *)(&(r7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r6 = _NL2Ltable[index - 3371];
                      } else {
                        r6 = _XILI_ROUND_U8(r7 * 56.61F);
                      }

                      if(g7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(g7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        g6 = _NL2Ltable[index - 3371];
                      } else {
                        g6 = _XILI_ROUND_U8(g7 * 56.61F);
                      }

                      if(b7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(b7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        b6 = _NL2Ltable[index - 3371];
                      } else {
                        b6 = _XILI_ROUND_U8(b7 * 56.61F);
                      }

                      ylinear = _XILI_L_TO_Ylinear(r6, g6, b6);

                      *dest_pixel = _XILI_ROUND_U8((float)ylinear);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y6, cb6, cr6, r7, g7, b7;
                          double ylinear;
                          Xil_unsigned8 r6, g6, b6;

                          _XILI_NORMALIZE_PHOTO_B(*(src_band),
                                                  *(src_band+1),
                                                  *(src_band+2),
                                                  &y6, &cb6, &cr6);

                          _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);

                          if(r7 >= 0.081F) {
                            int index;
                            index = (*(int *)(&(r7)) & FMASK) >> 13;

                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            r6 = _NL2Ltable[index - 3371];
                          } else {
                            r6 = _XILI_ROUND_U8(r7 * 56.61F);
                          }
    
                          if(g7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(g7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            g6 = _NL2Ltable[index - 3371];
                          } else {
                            g6 = _XILI_ROUND_U8(g7 * 56.61F);
                          }
    
                          if(b7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(b7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            b6 = _NL2Ltable[index - 3371];
                          } else {
                            b6 = _XILI_ROUND_U8(b7 * 56.61F);
                          }

                          ylinear = _XILI_L_TO_Ylinear(r6, g6, b6);

                          *dest_band = _XILI_ROUND_U8((float)ylinear);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::y601_to_rgblinear(XilOp*       op,
					                      XilRoi*      roi,
					                      XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float tmp;

                      tmp = _XILI_NORMALIZE_Y601_B(*(src_pixel));

                      if(tmp >= 0.081F) {
                        int index;

                        index = (*(int *)(&(tmp)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *dest_pixel = _NL2Ltable[index - 3371];
                      } else {
                        *dest_pixel = _XILI_ROUND_U8(tmp * 56.61F);
                      }

                      *(dest_pixel + 1) = *dest_pixel;
                      *(dest_pixel + 2) = *dest_pixel;

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float tmp;

                          tmp = _XILI_NORMALIZE_Y601_B(*(src_band));

                          if(tmp >= 0.081F) {
                            int index;

                            index = (*(int *)(&(tmp)) & FMASK) >> 13;

                            if(index < 3371) {
                              index = 3371;
                            }

                            if(index > 7176) {
                              index = 7176;
                            }

                            *dest_band = _NL2Ltable[index - 3371];
                          } else {
                            *dest_band = _XILI_ROUND_U8(tmp * 56.61F);
                          }

                          *(dest_band + 1) = *dest_band;
                          *(dest_band + 2) = *dest_band;
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::y601_to_cmy(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y601;
                      Xil_unsigned8 r;

                      y601 = _XILI_NORMALIZE_Y601_B(*(src_pixel));

                      if(y601 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(y601)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r = _NL2Ltable[index - 3371];
                      } else {
                        r = _XILI_ROUND_U8(y601 * 56.61F);
                      }

                      _XILI_L_TO_CMY_B(r,
                                       r,
                                       r,
                                       dest_pixel,
                                       dest_pixel+1,
                                       dest_pixel + 2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y601;
                          Xil_unsigned8 r;

                          y601 = _XILI_NORMALIZE_Y601_B(*(src_band));

                          if(y601 >= 0.081F) {
                            int index;

                            index = (*(int *)(&(y601)) & FMASK) >> 13;

                            if(index < 3371) {
                              index = 3371;
                            }

                            if(index > 7176) {
                              index = 7176;
                            }

                            r = _NL2Ltable[index - 3371];
                          } else {
                            r = _XILI_ROUND_U8(y601 * 56.61F);
                          }

                          _XILI_L_TO_CMY_B(r,
                                           r,
                                           r,
                                           dest_band,
                                           dest_band + 1,
                                           dest_band + 2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::y601_to_cmyk(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y601;
                      Xil_unsigned8 r;

                      y601 = _XILI_NORMALIZE_Y601_B(*(src_pixel));

                      if(y601 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(y601)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r = _NL2Ltable[index - 3371];
                      } else {
                        r = _XILI_ROUND_U8(y601 * 56.61F);
                      }

                      _XILI_L_TO_CMY_B(r,
                                       r,
                                       r,
                                       dest_pixel,
                                       dest_pixel + 1,
                                       dest_pixel + 2);

                      *(dest_pixel + 3) = XIL_MINBYTE;

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y601;
                          Xil_unsigned8 r;

                          y601 = _XILI_NORMALIZE_Y601_B(*(src_band));

                          if(y601 >= 0.081F) {
                            int index;

                            index = (*(int *)(&(y601)) & FMASK) >> 13;

                            if(index < 3371) {
                              index = 3371;
                            }

                            if(index > 7176) {
                              index = 7176;
                            }

                            r = _NL2Ltable[index - 3371];
                          } else {
                            r = _XILI_ROUND_U8(y601 * 56.61F);
                          }

                          _XILI_L_TO_CMY_B(r,
                                           r,
                                           r,
                                           dest_band,
                                           dest_band + 1,
                                           dest_band + 2);

                          *(dest_band + 3) = XIL_MINBYTE;
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::y601_to_ylinear(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y601;

                      y601 = _XILI_NORMALIZE_Y601_B(*(src_pixel));

                      if(y601 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(y601)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *dest_pixel = _NL2Ltable[index - 3371];
                      } else {
                        *dest_pixel = _XILI_ROUND_U8(y601 * 56.61F);
                      }

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y601;

                          y601 = _XILI_NORMALIZE_Y601_B(*(src_band));

                          if(y601 >= 0.081F) {
                            int index;

                            index = (*(int *)(&(y601)) & FMASK) >> 13;

                            if(index < 3371) {
                              index = 3371;
                            }

                            if(index > 7176) {
                              index = 7176;
                            }

                            *dest_band = _NL2Ltable[index - 3371];
                          } else {
                            *dest_band = _XILI_ROUND_U8(y601 * 56.61F);
                          }
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::y709_to_rgblinear(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y709;

                      y709 = _XILI_NORMALIZE_Y709_B(*(src_pixel));

                      if(y709 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(y709)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *dest_pixel = _NL2Ltable[index - 3371];
                      } else {
                        *dest_pixel = _XILI_ROUND_U8(y709 * 56.61F);
                      }

                      *(dest_pixel + 1) = *dest_pixel;
                      *(dest_pixel + 2) = *dest_pixel;

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y709;

                          y709 = _XILI_NORMALIZE_Y709_B(*(src_band));

                          if(y709 >= 0.081F) {
                            int index;

                            index = (*(int *)(&(y709)) & FMASK) >> 13;

                            if(index < 3371) {
                              index = 3371;
                            }

                            if(index > 7176) {
                              index = 7176;
                            }

                            *dest_band = _NL2Ltable[index - 3371];
                          } else {
                            *dest_band = _XILI_ROUND_U8(y709 * 56.61F);
                          }

                          *(dest_band + 1) = *dest_band;
                          *(dest_band + 2) = *dest_band;
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::y709_to_cmy(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y709;
                      Xil_unsigned8 r;

                      y709 = _XILI_NORMALIZE_Y709_B(*(src_pixel));

                      if(y709 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(y709)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r = _NL2Ltable[index - 3371];
                      } else {
                        r = _XILI_ROUND_U8(y709 * 56.61F);
                      }

                      _XILI_L_TO_CMY_B(r,
                                       r,
                                       r,
                                       dest_pixel,
                                       dest_pixel+1,
                                       dest_pixel + 2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y709;
                          Xil_unsigned8 r;

                          y709 = _XILI_NORMALIZE_Y709_B(*(src_band));

                          if(y709 >= 0.081F) {
                            int index;

                            index = (*(int *)(&(y709)) & FMASK) >> 13;

                            if(index < 3371) {
                              index = 3371;
                            }

                            if(index > 7176) {
                              index = 7176;
                            }

                            r = _NL2Ltable[index - 3371];
                          } else {
                            r = _XILI_ROUND_U8(y709 * 56.61F);
                          }

                          _XILI_L_TO_CMY_B(r,
                                           r,
                                           r,
                                           dest_band,
                                           dest_band + 1,
                                           dest_band + 2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::y709_to_cmyk(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y709;
                      Xil_unsigned8 r;

                      y709 = _XILI_NORMALIZE_Y709_B(*(src_pixel));

                      if(y709 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(y709)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r = _NL2Ltable[index - 3371];
                      } else {
                        r = _XILI_ROUND_U8(y709 * 56.61F);
                      }

                      _XILI_L_TO_CMY_B(r,
                                       r,
                                       r,
                                       dest_pixel,
                                       dest_pixel + 1,
                                       dest_pixel + 2);

                      *(dest_pixel + 3) = XIL_MINBYTE;

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y709;
                          Xil_unsigned8 r;

                          y709 = _XILI_NORMALIZE_Y709_B(*(src_band));

                          if(y709 >= 0.081F) {
                            int index;

                            index = (*(int *)(&(y709)) & FMASK) >> 13;

                            if(index < 3371) {
                              index = 3371;
                            }

                            if(index > 7176) {
                              index = 7176;
                            }

                            r = _NL2Ltable[index - 3371];
                          } else {
                            r = _XILI_ROUND_U8(y709 * 56.61F);
                          }

                          _XILI_L_TO_CMY_B(r,
                                           r,
                                           r,
                                           dest_band,
                                           dest_band + 1,
                                           dest_band + 2);

                          *(dest_band + 3) = XIL_MINBYTE;
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::y709_to_ylinear(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y709;

                      y709 = _XILI_NORMALIZE_Y709_B(*(src_pixel));

                      if(y709 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(y709)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *dest_pixel = _NL2Ltable[index - 3371];
                      } else {
                        *dest_pixel = _XILI_ROUND_U8(y709 * 56.61F);
                      }

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y709;

                          y709 = _XILI_NORMALIZE_Y709_B(*(src_band));

                          if(y709 >= 0.081F) {
                            int index;

                            index = (*(int *)(&(y709)) & FMASK) >> 13;

                            if(index < 3371) {
                              index = 3371;
                            }

                            if(index > 7176) {
                              index = 7176;
                            }

                            *dest_band = _NL2Ltable[index - 3371];
                          } else {
                            *dest_band = _XILI_ROUND_U8(y709 * 56.61F);
                          }
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::ycc601_to_rgblinear(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y6, cb6, cr6, red, green, blue;

                      _XILI_NORMALIZE_YCC601_B(*(src_pixel),
                                               *(src_pixel+1), 
                                               *(src_pixel+2), 
                                               &y6, &cb6, &cr6);

                      _XILI_YCC601_TO_NL(y6, cb6, cr6, &red, &green, &blue);

                      if(red >= 0.081F) {
                        int index;

                        index = (*(int *)(&(red)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *(dest_pixel + 2) = _NL2Ltable[index - 3371];
                      } else {
                        *(dest_pixel + 2) = _XILI_ROUND_U8(red * 56.61F);
                      }

                      if(green >= 0.081F) {
                        int index;

                        index = (*(int *)(&(green)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *(dest_pixel + 1) = _NL2Ltable[index - 3371];
                      } else {
                        *(dest_pixel + 1) = _XILI_ROUND_U8(green * 56.61F);
                      }

                      if(blue >= 0.081F) {
                        int index;

                        index = (*(int *)(&(blue)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *(dest_pixel) = _NL2Ltable[index - 3371];
                      } else {
                        *(dest_pixel) = _XILI_ROUND_U8(blue * 56.61F);
                      }

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y6, cb6, cr6, red, green, blue;

                          _XILI_NORMALIZE_YCC601_B(*(src_band),
                                                   *(src_band+1), 
                                                   *(src_band+2), 
                                                   &y6, &cb6, &cr6);
    
                          _XILI_YCC601_TO_NL(y6, cb6, cr6, &red, &green, &blue);
    
                          if(red >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(red)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            *(dest_band + 2) = _NL2Ltable[index - 3371];
                          } else {
                            *(dest_band + 2) = _XILI_ROUND_U8(red * 56.61F);
                          }
    
                          if(green >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(green)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            *(dest_band + 1) = _NL2Ltable[index - 3371];
                          } else {
                            *(dest_band + 1) = _XILI_ROUND_U8(green * 56.61F);
                          }
    
                          if(blue >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(blue)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            *(dest_band) = _NL2Ltable[index - 3371];
                          } else {
                            *(dest_band) = _XILI_ROUND_U8(blue * 56.61F);
                          }
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::ycc601_to_cmy(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y6, cb6, cr6, r7, g7, b7;
                      Xil_unsigned8 r6, g6, b6;

                      _XILI_NORMALIZE_YCC601_B(*(src_pixel),
                                               *(src_pixel+1), 
                                               *(src_pixel+2), 
                                               &y6, &cb6, &cr6);

                      _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);

                      if(r7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(r7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r6 = _NL2Ltable[index - 3371];
                      } else {
                        r6 = _XILI_ROUND_U8(r7 * 56.61F);
                      }

                      if(g7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(g7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        g6 = _NL2Ltable[index - 3371];
                      } else {
                        g6 = _XILI_ROUND_U8(g7 * 56.61F);
                      }

                      if(b7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(b7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        b6 = _NL2Ltable[index - 3371];
                      } else {
                        b6 = _XILI_ROUND_U8(b7 * 56.61F);
                      }

                      _XILI_L_TO_CMY_B(r6, g6, b6,
                                       dest_pixel,
                                       dest_pixel+1,
                                       dest_pixel+2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y6, cb6, cr6, r7, g7, b7;
                          Xil_unsigned8 r6, g6, b6;

                          _XILI_NORMALIZE_YCC601_B(*(src_band),
                                                   *(src_band+1), 
                                                   *(src_band+2), 
                                                   &y6, &cb6, &cr6);
    
                          _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);
    
                          if(r7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(r7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            r6 = _NL2Ltable[index - 3371];
                          } else {
                            r6 = _XILI_ROUND_U8(r7 * 56.61F);
                          }
    
                          if(g7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(g7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            g6 = _NL2Ltable[index - 3371];
                          } else {
                            g6 = _XILI_ROUND_U8(g7 * 56.61F);
                          }
    
                          if(b7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(b7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            b6 = _NL2Ltable[index - 3371];
                          } else {
                            b6 = _XILI_ROUND_U8(b7 * 56.61F);
                          }

                          _XILI_L_TO_CMY_B(r6, g6, b6,
                                           dest_band,
                                           dest_band+1,
                                           dest_band+2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::ycc601_to_cmyk(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y6, cb6, cr6, r7, g7, b7;
                      Xil_unsigned8 r6, g6, b6;

                      _XILI_NORMALIZE_YCC601_B(*(src_pixel),
                                               *(src_pixel+1), 
                                               *(src_pixel+2), 
                                               &y6, &cb6, &cr6);

                      _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);

                      if(r7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(r7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r6 = _NL2Ltable[index - 3371];
                      } else {
                        r6 = _XILI_ROUND_U8(r7 * 56.61F);
                      }

                      if(g7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(g7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        g6 = _NL2Ltable[index - 3371];
                      } else {
                        g6 = _XILI_ROUND_U8(g7 * 56.61F);
                      }

                      if(b7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(b7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        b6 = _NL2Ltable[index - 3371];
                      } else {
                        b6 = _XILI_ROUND_U8(b7 * 56.61F);
                      }

                      _XILI_L_TO_CMY_B(r6, g6, b6,
                                       dest_pixel,
                                       dest_pixel+1,
                                       dest_pixel+2);

                      *(dest_pixel + 3) = XIL_MINBYTE;

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y6, cb6, cr6, r7, g7, b7;
                          Xil_unsigned8 r6, g6, b6;

                          _XILI_NORMALIZE_YCC601_B(*(src_band),
                                                   *(src_band+1), 
                                                   *(src_band+2), 
                                                   &y6, &cb6, &cr6);
    
                          _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);
    
                          if(r7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(r7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            r6 = _NL2Ltable[index - 3371];
                          } else {
                            r6 = _XILI_ROUND_U8(r7 * 56.61F);
                          }
    
                          if(g7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(g7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            g6 = _NL2Ltable[index - 3371];
                          } else {
                            g6 = _XILI_ROUND_U8(g7 * 56.61F);
                          }
    
                          if(b7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(b7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            b6 = _NL2Ltable[index - 3371];
                          } else {
                            b6 = _XILI_ROUND_U8(b7 * 56.61F);
                          }

                          _XILI_L_TO_CMY_B(r6, g6, b6,
                                           dest_band,
                                           dest_band+1,
                                           dest_band+2);

                          *(dest_band + 3) = XIL_MINBYTE;
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::ycc601_to_ylinear(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y6, cb6, cr6, r7, g7, b7;
                      double  ylinear;
                      Xil_unsigned8 r6, g6, b6;

                      _XILI_NORMALIZE_YCC601_B(*(src_pixel),
                                               *(src_pixel + 1),
                                               *(src_pixel + 2),
                                               &y6, &cb6, &cr6);

                      _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);

                      if(r7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(r7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r6 = _NL2Ltable[index - 3371];
                      } else {
                        r6 = _XILI_ROUND_U8(r7 * 56.61F);
                      }

                      if(g7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(g7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        g6 = _NL2Ltable[index - 3371];
                      } else {
                        g6 = _XILI_ROUND_U8(g7 * 56.61F);
                      }

                      if(b7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(b7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        b6 = _NL2Ltable[index - 3371];
                      } else {
                        b6 = _XILI_ROUND_U8(b7 * 56.61F);
                      }

                      ylinear = _XILI_L_TO_Ylinear(r6, g6, b6);

                      *dest_pixel = _XILI_ROUND_U8(ylinear);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y6, cb6, cr6, r7, g7, b7;
                          double  ylinear;
                          Xil_unsigned8 r6, g6, b6;
    
                          _XILI_NORMALIZE_YCC601_B(*(src_band),
                                                   *(src_band + 1),
                                                   *(src_band + 2),
                                                   &y6, &cb6, &cr6);
    
                          _XILI_YCC601_TO_NL(y6, cb6, cr6, &r7, &g7, &b7);
    
                          if(r7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(r7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            r6 = _NL2Ltable[index - 3371];
                          } else {
                            r6 = _XILI_ROUND_U8(r7 * 56.61F);
                          }
    
                          if(g7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(g7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            g6 = _NL2Ltable[index - 3371];
                          } else {
                            g6 = _XILI_ROUND_U8(g7 * 56.61F);
                          }
    
                          if(b7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(b7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            b6 = _NL2Ltable[index - 3371];
                          } else {
                            b6 = _XILI_ROUND_U8(b7 * 56.61F);
                          }
    
                          ylinear = _XILI_L_TO_Ylinear(r6, g6, b6);
    
                          *dest_band = _XILI_ROUND_U8(ylinear);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::ycc709_to_rgblinear(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y709, cb, cr, red, green, blue;

                      _XILI_NORMALIZE_YCC709_B(*(src_pixel),
                                               *(src_pixel+1), 
                                               *(src_pixel+2), 
                                               &y709, &cb, &cr);

                      _XILI_YCC709_TO_NL(y709, cb, cr, &red, &green, &blue);

                      if(red >= 0.081F) {
                        int index;

                        index = (*(int *)(&(red)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *(dest_pixel + 2) = _NL2Ltable[index - 3371];
                      } else {
                        *(dest_pixel + 2) = _XILI_ROUND_U8(red * 56.61F);
                      }

                      if(green >= 0.081F) {
                        int index;

                        index = (*(int *)(&(green)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *(dest_pixel + 1) = _NL2Ltable[index - 3371];
                      } else {
                        *(dest_pixel + 1) = _XILI_ROUND_U8(green * 56.61F);
                      }

                      if(blue >= 0.081F) {
                        int index;

                        index = (*(int *)(&(blue)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        *(dest_pixel) = _NL2Ltable[index - 3371];
                      } else {
                        *(dest_pixel) = _XILI_ROUND_U8(blue * 56.61F);
                      }

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y709, cb, cr, red, green, blue;

                          _XILI_NORMALIZE_YCC709_B(*(src_band),
                                                   *(src_band+1), 
                                                   *(src_band+2), 
                                                   &y709, &cb, &cr);
    
                          _XILI_YCC709_TO_NL(y709, cb, cr, &red, &green, &blue);
    
                          if(red >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(red)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            *(dest_band + 2) = _NL2Ltable[index - 3371];
                          } else {
                            *(dest_band + 2) = _XILI_ROUND_U8(red * 56.61F);
                          }
    
                          if(green >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(green)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            *(dest_band + 1) = _NL2Ltable[index - 3371];
                          } else {
                            *(dest_band + 1) = _XILI_ROUND_U8(green * 56.61F);
                          }
    
                          if(blue >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(blue)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            *(dest_band) = _NL2Ltable[index - 3371];
                          } else {
                            *(dest_band) = _XILI_ROUND_U8(blue * 56.61F);
                          }
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::ycc709_to_cmy(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y709, cb, cr, r7, g7, b7;
                      Xil_unsigned8 r6, g6, b6;

                      _XILI_NORMALIZE_YCC709_B(*(src_pixel),
                                               *(src_pixel+1), 
                                               *(src_pixel+2), 
                                               &y709, &cb, &cr);

                      _XILI_YCC709_TO_NL(y709, cb, cr, &r7, &g7, &b7);

                      if(r7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(r7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r6 = _NL2Ltable[index - 3371];
                      } else {
                        r6 = _XILI_ROUND_U8(r7 * 56.61F);
                      }

                      if(g7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(g7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        g6 = _NL2Ltable[index - 3371];
                      } else {
                        g6 = _XILI_ROUND_U8(g7 * 56.61F);
                      }

                      if(b7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(b7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        b6 = _NL2Ltable[index - 3371];
                      } else {
                        b6 = _XILI_ROUND_U8(b7 * 56.61F);
                      }

                      _XILI_L_TO_CMY_B(r6, g6, b6,
                                       dest_pixel,
                                       dest_pixel+1,
                                       dest_pixel+2);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y709, cb, cr, r7, g7, b7;
                          Xil_unsigned8 r6, g6, b6;

                          _XILI_NORMALIZE_YCC709_B(*(src_band),
                                                   *(src_band+1), 
                                                   *(src_band+2), 
                                                   &y709, &cb, &cr);
    
                          _XILI_YCC709_TO_NL(y709, cb, cr, &r7, &g7, &b7);
    
                          if(r7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(r7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            r6 = _NL2Ltable[index - 3371];
                          } else {
                            r6 = _XILI_ROUND_U8(r7 * 56.61F);
                          }
    
                          if(g7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(g7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            g6 = _NL2Ltable[index - 3371];
                          } else {
                            g6 = _XILI_ROUND_U8(g7 * 56.61F);
                          }
    
                          if(b7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(b7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            b6 = _NL2Ltable[index - 3371];
                          } else {
                            b6 = _XILI_ROUND_U8(b7 * 56.61F);
                          }

                          _XILI_L_TO_CMY_B(r6, g6, b6,
                                           dest_band,
                                           dest_band+1,
                                           dest_band+2);
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::ycc709_to_cmyk(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y709, cb, cr, r7, g7, b7;
                      Xil_unsigned8 r6, g6, b6;

                      _XILI_NORMALIZE_YCC709_B(*(src_pixel),
                                               *(src_pixel+1), 
                                               *(src_pixel+2), 
                                               &y709, &cb, &cr);

                      _XILI_YCC709_TO_NL(y709, cb, cr, &r7, &g7, &b7);

                      if(r7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(r7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r6 = _NL2Ltable[index - 3371];
                      } else {
                        r6 = _XILI_ROUND_U8(r7 * 56.61F);
                      }

                      if(g7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(g7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        g6 = _NL2Ltable[index - 3371];
                      } else {
                        g6 = _XILI_ROUND_U8(g7 * 56.61F);
                      }

                      if(b7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(b7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        b6 = _NL2Ltable[index - 3371];
                      } else {
                        b6 = _XILI_ROUND_U8(b7 * 56.61F);
                      }

                      _XILI_L_TO_CMY_B(r6, g6, b6,
                                       dest_pixel,
                                       dest_pixel+1,
                                       dest_pixel+2);

                      *(dest_pixel + 3) = XIL_MINBYTE;

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y709, cb, cr, r7, g7, b7;
                          Xil_unsigned8 r6, g6, b6;

                          _XILI_NORMALIZE_YCC709_B(*(src_band),
                                                   *(src_band+1), 
                                                   *(src_band+2), 
                                                   &y709, &cb, &cr);
    
                          _XILI_YCC709_TO_NL(y709, cb, cr, &r7, &g7, &b7);
    
                          if(r7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(r7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            r6 = _NL2Ltable[index - 3371];
                          } else {
                            r6 = _XILI_ROUND_U8(r7 * 56.61F);
                          }
    
                          if(g7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(g7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            g6 = _NL2Ltable[index - 3371];
                          } else {
                            g6 = _XILI_ROUND_U8(g7 * 56.61F);
                          }
    
                          if(b7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(b7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            b6 = _NL2Ltable[index - 3371];
                          } else {
                            b6 = _XILI_ROUND_U8(b7 * 56.61F);
                          }

                          _XILI_L_TO_CMY_B(r6, g6, b6,
                                           dest_band,
                                           dest_band+1,
                                           dest_band+2);

                          *(dest_band + 3) = XIL_MINBYTE;
                        }

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

XilStatus
XilDeviceManagerComputeBYTE::ycc709_to_ylinear(XilOp*       op,
                                          XilRoi*      roi,
                                          XilBoxList*  bl)
{
    //
    //  Get the images for our operation.
    //
    XilImage* src = op->getSrcImage(1);
    XilImage* dest = op->getDstImage(1);

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
            Xil_unsigned8* src_data;
            src_storage.getStorageInfo(&src_pixel_stride,
                                           &src_scanline_stride,
                                           NULL, NULL,
                                           (void**)&src_data);

            unsigned int   dest_pixel_stride;
            unsigned int   dest_scanline_stride;
            Xil_unsigned8* dest_data;
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

                Xil_unsigned8* src_scanline = src_data +
                        (y*src_scanline_stride) + (x*src_pixel_stride);

                Xil_unsigned8* dest_scanline = dest_data +
                        (y*dest_scanline_stride) + (x*dest_pixel_stride);

                do { // each scanline

                    Xil_unsigned8* src_pixel = src_scanline;
                    Xil_unsigned8* dest_pixel = dest_scanline;

                    for (unsigned int i=0; i<xsize; i++) {
                      float y709, cb, cr, r7, g7, b7;
                      double  ylinear;
                      Xil_unsigned8 r6, g6, b6;

                      _XILI_NORMALIZE_YCC709_B(*(src_pixel),
                                               *(src_pixel + 1),
                                               *(src_pixel + 2),
                                               &y709, &cb, &cr);

                      _XILI_YCC709_TO_NL(y709, cb, cr, &r7, &g7, &b7);

                      if(r7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(r7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        r6 = _NL2Ltable[index - 3371];
                      } else {
                        r6 = _XILI_ROUND_U8(r7 * 56.61F);
                      }

                      if(g7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(g7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        g6 = _NL2Ltable[index - 3371];
                      } else {
                        g6 = _XILI_ROUND_U8(g7 * 56.61F);
                      }

                      if(b7 >= 0.081F) {
                        int index;

                        index = (*(int *)(&(b7)) & FMASK) >> 13;

                        if(index < 3371) {
                          index = 3371;
                        }

                        if(index > 7176) {
                          index = 7176;
                        }

                        b6 = _NL2Ltable[index - 3371];
                      } else {
                        b6 = _XILI_ROUND_U8(b7 * 56.61F);
                      }

                      ylinear = _XILI_L_TO_Ylinear(r6, g6, b6);

                      *dest_pixel = _XILI_ROUND_U8(ylinear);

                      src_pixel += src_pixel_stride;
                      dest_pixel += dest_pixel_stride;
                    }

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
            Xil_unsigned8* src_data[_XILI_CS_MAX_BANDS];
            unsigned int src_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int src_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < src_nbands; b++) {
                src_data[b] = (Xil_unsigned8*)src_storage.getDataPtr(b);
                src_pixel_stride[b] = src_storage.getPixelStride(b);
                src_scanline_stride[b] = src_storage.getScanlineStride(b);
            }

            Xil_unsigned8* dest_data[_XILI_CS_MAX_BANDS];
            unsigned int dest_pixel_stride[_XILI_CS_MAX_BANDS];
            unsigned int dest_scanline_stride[_XILI_CS_MAX_BANDS];

            for(b = 0; b < dest_nbands; b++) {
                dest_data[b] = (Xil_unsigned8*)dest_storage.getDataPtr(b);
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

                        Xil_unsigned8 src_band[_XILI_CS_MAX_BANDS];

                        for(b = 0; b < src_nbands; b++) {
                            src_band[b] = *(src_data[b] +
                                line * src_scanline_stride[b] +
                                pixel * src_pixel_stride[b]);
                        }

                        Xil_unsigned8 dest_band[_XILI_CS_MAX_BANDS];

                        for (unsigned int i=0; i<1; i++) {
                          float y709, cb, cr, r7, g7, b7;
                          double  ylinear;
                          Xil_unsigned8 r6, g6, b6;
    
                          _XILI_NORMALIZE_YCC709_B(*(src_band),
                                                   *(src_band + 1),
                                                   *(src_band + 2),
                                                   &y709, &cb, &cr);
    
    
                          _XILI_YCC709_TO_NL(y709, cb, cr, &r7, &g7, &b7);

                          if(r7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(r7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            r6 = _NL2Ltable[index - 3371];
                          } else {
                            r6 = _XILI_ROUND_U8(r7 * 56.61F);
                          }
    
                          if(g7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(g7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            g6 = _NL2Ltable[index - 3371];
                          } else {
                            g6 = _XILI_ROUND_U8(g7 * 56.61F);
                          }
    
                          if(b7 >= 0.081F) {
                            int index;
    
                            index = (*(int *)(&(b7)) & FMASK) >> 13;
    
                            if(index < 3371) {
                              index = 3371;
                            }
    
                            if(index > 7176) {
                              index = 7176;
                            }
    
                            b6 = _NL2Ltable[index - 3371];
                          } else {
                            b6 = _XILI_ROUND_U8(b7 * 56.61F);
                          }
    
                          ylinear = _XILI_L_TO_Ylinear(r6, g6, b6);
    
                          *dest_band = _XILI_ROUND_U8(ylinear);
                        }

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

