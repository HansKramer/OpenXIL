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
//  File:       ComputeInfoMMX.cc
//  Project:    XIL
//  Revision:   1.15
//  Last Mod:   10:13:42, 03/10/00
//
//  Description:
//
//    Compute Info class for the MMX pipeline
//    Contains methods to set up IplImage structures
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)ComputeInfoMMX.cc	1.15\t00/03/10  "

#include "ComputeInfoMMX.hh"

ComputeInfoMMX::ComputeInfoMMX(XilOp*       init_op,
                               unsigned int init_op_count,
                               XilRoi*      init_roi,
                               XilBoxList*  init_bl) 
 : ComputeInfoGENERAL(init_op, init_op_count, init_roi, init_bl)
{

    if(! isOK()) {
        return;
    }

    //
    // Loop through all of the dst and src images
    // and fill in the IplImage structures with 
    // those parameters which won't change on a rect-by-rect basis.
    //
    for(unsigned int i=0; i<=numSrcs; i++) {
        //
        // Get the image specific information from each of the images
        //
        int            nbands;
        IplImage*      mmxImage;

        switch(i) {
          case 0:
            if(numDsts > 0) {
                nbands    = destNumBands; 
                mmxImage  = &mmxDst;
            } else {
                //
                // No destination image. Skip image setup.
                // (Ops like histogram have no destination image).
                //
                continue;
            }
            break;

          case 1:
            nbands     = src1NumBands; 
            mmxImage   = &mmxSrc1;
            break;

          case 2:
            nbands     = src2NumBands; 
            mmxImage   = &mmxSrc2;
            break;

          case 3:
            nbands     = src3NumBands; 
            mmxImage   = &mmxSrc3;
            break;
          default:
            // Can't happen
            return;
        }

        //
        // Select the color model based on the number of bands
        //
        char* colorModel;
        switch(nbands) {
          case 1:
            colorModel = "GRAY";
            break;
          case 3:
          case 4:
            colorModel = "RGB ";
            break;
          default:
            colorModel = "MSI ";
            break;
        }

        //
        // Fill in the image constant parameters,
        // i.e.those which don;t vary with rect
        //
        mmxImage->nSize           = sizeof(IplImage);
        mmxImage->ID              = 1;  // Aplication handle (not used)
        mmxImage->nChannels       = nbands;
        mmxImage->alphaChannel    = 0;
        mmxImage->depth           = IPL_DEPTH_8U;
        strncpy(mmxImage->colorModel, colorModel, 4);
        strncpy(mmxImage->channelSeq, "BGR ", 4);
        mmxImage->dataOrder       = IPL_DATA_ORDER_PIXEL;
        mmxImage->origin          = IPL_ORIGIN_TL;
        mmxImage->align           = IPL_ALIGN_QWORD;
        mmxImage->imageId         = NULL;
        mmxImage->tileInfo        = NULL;
        mmxImage->BorderMode[0]   = IPL_SIDE_TOP; 
        mmxImage->BorderMode[1]   = IPL_SIDE_BOTTOM; 
        mmxImage->BorderMode[2]   = IPL_SIDE_LEFT; 
        mmxImage->BorderMode[3]   = IPL_SIDE_RIGHT; 
        mmxImage->BorderConst[0]  = 0;
        mmxImage->BorderConst[1]  = 0;
        mmxImage->BorderConst[2]  = 0;
        mmxImage->BorderConst[3]  = 0;
        mmxImage->imageDataOrigin = NULL;

    } // End src image loop

}

//
// Destructor
//
ComputeInfoMMX::~ComputeInfoMMX(void)
{
}

XilStatus
ComputeInfoMMX::createIplImages(Xil_boolean failOnMisaligned)
{
    //
    // Loop through all of the dst and src images
    // and fill in the remainder of the IplImage structures based on
    // the image and rect descriptions.
    //
    for(unsigned int i=0; i<=numSrcs; i++) {
        //
        // Get the image specific information from each of the images
        //
        int            sstride;
        int            pstride;
        char*          rectPtr;
        IplImage*      mmxImage;
        IplROI*        mmxRoi;

        switch(i) {
          case 0:
            if(numDsts > 0) {
                sstride   = destScanlineStride;
                pstride   = destPixelStride;
                rectPtr   = (char*)destData;
                mmxImage  = &mmxDst;
                mmxRoi    = &mmxDstRoi;
            } else {
                //
                // No destination image. Skip image setup.
                // (Ops like histogram have no destination image).
                //
                continue;
            }
            break;

          case 1:
            sstride  = src1ScanlineStride;
            pstride  = src1PixelStride;
            rectPtr  = (char*)src1Data;
            mmxImage = &mmxSrc1;
            mmxRoi   = &mmxSrc1Roi;
            break;

          case 2:
            sstride  = src2ScanlineStride;
            pstride  = src2PixelStride;
            rectPtr  = (char*)src2Data;
            mmxImage = &mmxSrc2;
            mmxRoi   = &mmxSrc2Roi;
            break;

          case 3:
            sstride  = src3ScanlineStride;
            pstride  = src3PixelStride;
            rectPtr  = (char*)src3Data;
            mmxImage = &mmxSrc3;
            mmxRoi   = &mmxSrc3Roi;
            break;
          default:
            // Can't happen
            return XIL_FAILURE;
        }

        //
        // Create an IplROI structure to describe the rect to be processed
        // Note:
        //
        //   The IPL library is designed to process complete images
        //   from its API, just like XIL. The way to restrict it to
        //   subsets is to create an IplROI which describes the
        //   rectangle to be processed. This lets our GPI compute
        //   routines, which generally receive rectangles, send
        //   processing requests to the IPL library.
        //
        //   However, we need to perform a bit of trickery here.
        //   IPL wants images 64bit aligned for best MMX performance.
        //   For full images, we can do that, and we have modified the
        //   storage device to make sure the scanline stride is a 
        //   multiple of 8. However the XIL GPI interface supplies boxes
        //   (and ultimately rects) which are arbitrarily aligned. 
        //
        //   So the solution used here is to make the IPL library
        //   think its getting an aligned image by moving the data
        //   pointer back to where it is 64 bit aligned. Then we
        //   specify the rect as a ROI relative to this alignment point.
        //   Real data should actually exist at the alignment point, so
        //   there should be no chance of an access violation.
        //
        //   This is only required if the rectangle to be processed
        //   is not aligned on a 64 bit boundary.
        //   
        //   Another bit of chicanery is the necessity to convince IPL
        //   that the image width is directly related to the scan stride,
        //   since they appear to do some internal checking on this.
        //   So we make the image width be the same as the scan stride
        //   divided by the number of bands.
        //


        //
        // Determine the alignment of the rect
        //
        unsigned int offset8 = ((unsigned long)rectPtr & 0x7);
        if(failOnMisaligned && (offset8 != 0)) {
            return XIL_FAILURE;
        }

        //
        // Since the ComputeInfo interface has already calculated
        // the dataPtr for the ULC of the rectangle, our rect
        // origin must be set at (0,0), since the IplROI will be
        // relative to the start of our "image".
        //
        int x_rect = 0;
        int y_rect = 0;
        int w_rect = xsize;
        int h_rect = ysize;

        //
        // Pretend the image is the same width as the stride.
        // (This seems to be necesary to get Ipl to behave properly)
        //
        int w_image = sstride / mmxImage->nChannels;

        //
        // Make the IplImage "height" that of the rect.
        //
        int h_image = ysize;

        //
        // Check the pixel stride against the number of bands,
        // as a means of detecting child images.
        // Since XIL can't tell us which band this is,
        // we can't use Ipl's channel-of-interest field
        // to restrict processing to a single band.
        // Bummer. Punt to memory.
        //
        Xil_boolean  isChild = (mmxImage->nChannels != pstride);
        if(isChild) {
           return XIL_FAILURE;
        }

        //
        // Use a ROI to describe the rectangle.
        //
        iplSetROI(mmxRoi, 0, x_rect, y_rect, h_rect, w_rect);

        mmxImage->height    = h_image;
        mmxImage->width     = w_image;
        mmxImage->roi       = mmxRoi;
        mmxImage->imageSize = sstride * h_image;
        mmxImage->imageData = rectPtr;
        mmxImage->widthStep = sstride;

    } // End src image loop

    return XIL_SUCCESS;
}


XilBox*
ComputeInfoMMX::getSrc1Box()
{
    return boxes[src1BoxesOffset];
}

XilBox*
ComputeInfoMMX::getDestBox()
{
    return boxes[destBoxesOffset];
}

XilBoxAreaType
ComputeInfoMMX::getBoxTag()
{
    return (XilBoxAreaType)((int)boxes[destBoxesOffset]->getTag());
}
