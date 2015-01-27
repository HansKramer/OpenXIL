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
//  File:       decompress.cc
//  Project:    XIL
//  Revision:   1.15
//  Last Mod:   10:14:55, 03/10/00
//
//  Description:
//    Decompress a frame to memory.  Scale subsampled bands to produce
//    a uniformly sampled image.
//
//  Returns:
//        XIL_SUCCESS or XIL_FAILURE
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompress.cc	1.15\t00/03/10  "

#include <string.h>
#include "xil/xilGPI.hh"
#include "XilDeviceCompressionMpeg1.hh"
#include "XilDeviceManagerCompressionMpeg1.hh"
#include "Mpeg1DecompressorData.hh"
#include "Mpeg1Decoder.hh"
#include "IdctRefFrame.hh"

XilStatus
XilDeviceCompressionMpeg1::decompress(XilOp*       op,
                                      unsigned int op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl)
{
    //
    // Create an Info object to contain decompression parameters
    //
    DecompressInfo di(op, op_count, roi, bl);
    if(! di.isOK()) {
        return XIL_FAILURE;
    }

    //
    // Create a Block Handling object
    //
    BlockMan blok;

    setInMolecule(FALSE);

    // 
    // Seek to the correct frame
    //
    seek(di.frame_number);

    Mpeg1DecompressorData* decoder = getMpeg1DecompressorData();

    Mpeg1ReferenceFrame* tmp_frame;

    // 
    // Check history buffers in case skipped frames, setup at proper read 
    // position
    //
    int status = decoder->checkHistoryBuffers();
    if(status == 0) {
        //
        // Use the results that are in the history buffer ("last")
        //
        tmp_frame = decoder->last;
    } else {
        //
        // Decode a new frame from the bitstream
        //
        if( !decoder->setByteStreamPtr()) {

            //
            // We have a condition where we get no frame from buffer mgr
            // However, we are reporting: Mpeg1 bytestream error.
            // (should we tell users more?)
            //
            XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-98", TRUE);
            return XIL_FAILURE;
        }

        //
        // Actually decode the frame
        //
        decoder->color_video_frame();
        decoder->finishDecode();

        //
        // I or P frames end up in decoder->next.
        // B frames end up in decoder->bbbb.
        //
        if(decoder->frametypeC == MPEG1_IFRAME_TYPE || 
           decoder->frametypeC == MPEG1_PFRAME_TYPE) {
            tmp_frame = decoder->next;
        } else { // B frame
            tmp_frame = decoder->bbbb;
        }
    }

    if(! tmp_frame->getFilledFlag()) {
        tmp_frame->populateReference();
    }

    //
    // The frame is actually decompresed at this point, but the chroma
    // upsampling remains to be done. This is done either directly to 
    // the destination image or, if ROIs are involved, to a temporary
    //
    if(cisFitsInDst(roi)) {

        blok.upsample411FrameFromRaster(&di,
                                        tmp_frame->getYDataPtr(),
                                        tmp_frame->getYScanlineStride(),
                                        tmp_frame->getCbDataPtr(),
                                        tmp_frame->getCbScanlineStride(),
                                        tmp_frame->getCrDataPtr(),
                                        tmp_frame->getCrScanlineStride());

    } else {
        //
        // Construct a temporary buffer, with parameters held
        // in another DecompressInfo object.
        //
        DecompressInfo tmp_di(&di);
        if(! tmp_di.isOK()) {
            return XIL_FAILURE;
        }

        blok.upsample411FrameFromRaster(&tmp_di,
                                        tmp_frame->getYDataPtr(),
                                        tmp_frame->getYScanlineStride(),
                                        tmp_frame->getCbDataPtr(),
                                        tmp_frame->getCbScanlineStride(),
                                        tmp_frame->getCrDataPtr(),
                                        tmp_frame->getCrScanlineStride());

        //
        // Use the copyRects utility to handle miltiple dst rects
        //
        di.copyRects(&tmp_di);
    }

    return XIL_SUCCESS;
}


XilStatus
XilDeviceCompressionMpeg1::decompressColorConvert(XilOp*       op,
                                                  unsigned int op_count,
                                                  XilRoi*      roi,
                                                  XilBoxList*  bl)
{
    DecompressInfo di(op, op_count, roi, bl);
    if(! di.isOK()) {
        return XIL_FAILURE;
    }

    //
    // Create a Block Handling object
    //
    BlockMan blok;

    di.doColorConvert = TRUE;

    setInMolecule(TRUE);

    //
    // Get the op_list for determining molecule components.
    // The cis will always be on op_list[op_count-1].
    //
    XilOp**   op_list = op->getOpList();

    //
    // Verify that the parameters are acceptable. If not,
    // fail back to atomic mode.
    //
    if(! validDecompressColorConvert(op_list[0]->getSrcImage(),
                                     op_list[0]->getDstImage())) {
        return XIL_FAILURE;
    }

    //
    // Get the Ycc2RgbConverter object (creates it on first call)
    // This will initialize the tables which speed the color conversion.
    //
    XilDeviceManagerCompressionMpeg1* dmc = 
        (XilDeviceManagerCompressionMpeg1*)getDeviceManager();
    converter = dmc->getColorConverter();
    if(converter == NULL) {
        return XIL_FAILURE;
    }

    //
    // Pass the conversion object via the DecompressInfo object
    //
    di.objectPtr1 = (void*)converter;

    // 
    // Seek to the correct frame
    //
    seek(di.frame_number);

    Mpeg1DecompressorData* decoder = getMpeg1DecompressorData();

    // 
    // Check history buffers in case skipped frames, setup at proper read 
    // position
    //
    int status = decoder->checkHistoryBuffers();


    Mpeg1ReferenceFrame* tmp_frame;
    if(status == 0) {
        tmp_frame = decoder->last;
    } else {
        if( !decoder->setByteStreamPtr()) {

            //
            // We have a condition where 
            //   decompress: no frame from buffer mgr
            // However, we are reporting: Mpeg1 bytestream error.
            // (should we tell users more?)
            //
            XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-98", TRUE);
            return XIL_FAILURE;
        }

        decoder->color_video_frame();
        decoder->finishDecode();

        //
        // If I or P frame, in decoder->next, else in  decoder->bbbb
        //
        if(decoder->frametypeC == MPEG1_IFRAME_TYPE || 
           decoder->frametypeC == MPEG1_PFRAME_TYPE) {
            tmp_frame = decoder->next;
        } else { // B frame
            tmp_frame = decoder->bbbb;
        }
    }

    if(! tmp_frame->getFilledFlag()) {
        tmp_frame->populateReference();
    }

    //
    // The frame is actually decompresed at this point, but the chroma
    // upsampling remains to be done. This is done either directly to 
    // the destination image or, if ROIs are involved, to a temporary
    //
    if(cisFitsInDst(roi)) {


        blok.upsample411FrameFromRaster(&di,
                                        tmp_frame->getYDataPtr(),
                                        tmp_frame->getYScanlineStride(),
                                        tmp_frame->getCbDataPtr(),
                                        tmp_frame->getCbScanlineStride(),
                                        tmp_frame->getCrDataPtr(),
                                        tmp_frame->getCrScanlineStride());

    } else {
        //
        // Construct a temporary buffer, with parameters held
        // in another DecompressInfo object.
        //
        DecompressInfo tmp_di(&di);
        if(! tmp_di.isOK()) {
            return XIL_FAILURE;
        }

        blok.upsample411FrameFromRaster(&tmp_di,
                                        tmp_frame->getYDataPtr(),
                                        tmp_frame->getYScanlineStride(),
                                        tmp_frame->getCbDataPtr(),
                                        tmp_frame->getCbScanlineStride(),
                                        tmp_frame->getCrDataPtr(),
                                        tmp_frame->getCrScanlineStride());

        //
        // Use the copyRects utility to handle multiple dst rects
        //
        di.copyRects(&tmp_di);
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceCompressionMpeg1::decompressOrderedDither(XilOp*       op,
                                                   unsigned int op_count,
                                                   XilRoi*      roi,
                                                   XilBoxList*  bl)
{
    float scale[3]  = {1.0F, 1.0F, 1.0F };
    float offset[3] = {0.0F, 0.0F, 0.0F };

    DecompressInfo di(op, op_count, roi, bl);
    if(! di.isOK()) {
        return XIL_FAILURE;
    }

    //
    // Create a Block Handling object
    //
    BlockMan blok;

    di.doOrderedDither = TRUE;

    setInMolecule(TRUE);

    //
    // Get the op_list for determining molecule components.
    // The cis will always be on op_list[op_count-1].
    //
    XilOp**   op_list = op->getOpList();

    //
    // Verify that the parameters are acceptable. If not,
    // fail back to atomic mode.
    //
    XilLookupColorcube* cube;
    XilDitherMask*      dmask;
    op_list[0]->getParam(1, (XilObject**)&cube);
    op_list[0]->getParam(2, (XilObject**)&dmask);

    if(! validDecompressOrderedDither(&di, cube, dmask)) {
        return XIL_FAILURE;
    }

    //
    // Check if the op_count is 3. If it is, then this is the
    // decompressRescaleDither molecule.
    // So load the rescale params into the scale/offset arrays.
    //
    if(op_count == 3) {
        float*              xscale;
        float*              xoffset;
        op_list[1]->getParam(1, (void**)&xscale);
        op_list[1]->getParam(2, (void**)&xoffset);
        memcpy(scale, xscale, 3*sizeof(float));
        memcpy(offset, xoffset, 3*sizeof(float));
    }


    //
    // Get the XiliOrderedDitherLut object (creates it on first call)
    // This will initialize the tables which speed ordered dither.
    // This is passed into the DecompressInfo object as a generic ptr.
    //
    di.objectPtr1 = getDitherTable(cube, dmask, scale, offset);
    if(di.objectPtr1 == NULL) {
        return XIL_FAILURE;
    }

    // 
    // Seek to the correct frame
    //
    seek(di.frame_number);

    Mpeg1DecompressorData* decoder = getMpeg1DecompressorData();

    // 
    // Check history buffers in case skipped frames, setup at proper read 
    // position
    //
    int status = decoder->checkHistoryBuffers();


    Mpeg1ReferenceFrame* tmp_frame;
    if(status == 0) {
        tmp_frame = decoder->last;
    } else {
        if( !decoder->setByteStreamPtr()) {

            //
            // We have a condition where 
            //   decompress: no frame from buffer mgr
            // However, we are reporting: Mpeg1 bytestream error.
            // (should we tell users more?)
            //
            XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-98", TRUE);
            return XIL_FAILURE;
        }

        decoder->color_video_frame();
        decoder->finishDecode();

        //
        // If I or P frame, in decoder->next, else in  decoder->bbbb
        //
        if(decoder->frametypeC == MPEG1_IFRAME_TYPE || 
           decoder->frametypeC == MPEG1_PFRAME_TYPE) {
            tmp_frame = decoder->next;
        } else { // B frame
            tmp_frame = decoder->bbbb;
        }
    }

    if(! tmp_frame->getFilledFlag()) {
        tmp_frame->populateReference();
    }

    //
    // The frame is actually decompresed at this point, but the chroma
    // upsampling remains to be done. This is done either directly to 
    // the destination image or, if ROIs are involved, to a temporary
    //
    if(cisFitsInDst(roi)) {


        blok.upsample411FrameFromRaster(&di,
                                        tmp_frame->getYDataPtr(),
                                        tmp_frame->getYScanlineStride(),
                                        tmp_frame->getCbDataPtr(),
                                        tmp_frame->getCbScanlineStride(),
                                        tmp_frame->getCrDataPtr(),
                                        tmp_frame->getCrScanlineStride());

    } else {
        //
        // Construct a temporary buffer, with parameters held
        // in another DecompressInfo object.
        //
        DecompressInfo tmp_di(&di);
        if(! tmp_di.isOK()) {
            return XIL_FAILURE;
        }

        blok.upsample411FrameFromRaster(&tmp_di,
                                        tmp_frame->getYDataPtr(),
                                        tmp_frame->getYScanlineStride(),
                                        tmp_frame->getCbDataPtr(),
                                        tmp_frame->getCbScanlineStride(),
                                        tmp_frame->getCrDataPtr(),
                                        tmp_frame->getCrScanlineStride());

        //
        // Use the copyRects utility to handle multiple dst rects
        //
        di.copyRects(&tmp_di);
    }

    return XIL_SUCCESS;
}

//
// Verify that the constraints for decompressColorConvert Molecules
// are satisfied (ycc601->rgb709 color cvt, 16X dimensions)
//
Xil_boolean
XilDeviceCompressionMpeg1::validDecompressColorConvert(XilImage* src,
                                                      XilImage* dst)
{
    //
    // Color spaces must be Ycc601 src, Rgb709 dst
    //
    XilColorspace* src_cspace = src->refColorspace();
    XilColorspace* dst_cspace = dst->refColorspace();

    XilSystemState* state = getSystemState();
    XilColorspace*  desired_src_cspace =
        (XilColorspace*)state->getObjectByName("ycc601", XIL_COLORSPACE);
    XilColorspace* desired_dst_cspace =
        (XilColorspace*)state->getObjectByName("rgb709", XIL_COLORSPACE);

    if(src_cspace->getOpcode() != desired_src_cspace->getOpcode() ||
       dst_cspace->getOpcode() != desired_dst_cspace->getOpcode()) {
         return FALSE;
    }

    return TRUE;
}

//
// Verify that the constraints for decompressOrderedDither
// are satisfied (4x4 dither mask, 855 colorcube)
//
Xil_boolean
XilDeviceCompressionMpeg1::validDecompressOrderedDither(
          DecompressInfo*     di,
          XilLookupColorcube* cube,
          XilDitherMask*      dmask)
{
    //
    // Dithermask must be 4x4 3-band, but contents don't matter
    //
    if(dmask->getWidth()    != 4 ||
       dmask->getHeight()   != 4 ||
       dmask->getNumBands() != 3) {
        return FALSE;
    }

    //
    // Dst image must be a simple one-band image with
    // a pixel stride of 1 . 
    //
    if(di->image_ps != 1) {
        return XIL_FAILURE;
    }

    //
    // Colorcube must be the 855 variety, suitable for yuv2rgb conversion
    //

    const int*          mults = cube->getMultipliers();
    const unsigned int* dims  = cube->getDimensions();
    if(dims[0] != 8 || dims[1] != 5 || dims[2] != 5 ||
       mults[0] != 1 || mults[1] != 8 || mults[2] != 40) {
        return FALSE;
    }

    return TRUE;
}

