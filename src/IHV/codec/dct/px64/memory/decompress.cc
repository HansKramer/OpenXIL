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
//  Revision:   1.17
//  Last Mod:   10:15:15, 03/10/00
//
//  Description:
//
//    Perform H.261 frame decompression.
//    Upsamples the chroma bands.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompress.cc	1.17\t00/03/10  "


#include <stdio.h>
#include <xil/xilGPI.hh>
#include <string.h>
#include "XilDeviceManagerCompressionH261.hh"
#include "XilDeviceCompressionH261.hh"
#include "H261DecompressorData.hh"

XilStatus
XilDeviceCompressionH261::decompress(XilOp*       op,
                                     unsigned int op_count,
                                     XilRoi*      roi,
                                     XilBoxList*  bl)
{
    //
    // Construct the utility object to get all
    // size and storage data
    //
    DecompressInfo di(op, op_count, roi, bl);
    if(! di.isOK()) {
        return XIL_FAILURE;
    }

    setInMolecule(FALSE);

    //
    // Seek to the proper frame. Deferred execution may have
    // caused other frames to be decompressed before this one.
    //
    seekFlush(di.frame_number);

    //
    // Decoder object to hold info specific to this frame
    //
    H261DecompressorData* decoder = getH261DecompressorData();

    //
    // Read the bitstream header
    //
    if (!decoder->setByteStreamPtr()) {
      //
      // Decompress internal error: no frame from buffer mgr
      //
      XIL_ERROR(system_state, XIL_ERROR_SYSTEM,"di-95",FALSE);
      return XIL_FAILURE;
    }

    //
    // Decompress the frame into the internal buffers
    // held in the H261DecompresssorData class
    //
    decoder->color_video_frame(&di);
    decoder->finishDecode();

    return XIL_SUCCESS; 
}

XilStatus
XilDeviceCompressionH261::decompressColorConvert(XilOp*       op,
                                                 unsigned int op_count,
                                                 XilRoi*      roi,
                                                 XilBoxList*  bl)
{
    //
    // Construct the utility object to get all
    // size and storage data
    //
    DecompressInfo di(op, op_count, roi, bl);
    if(! di.isOK()) {
        return XIL_FAILURE;
    }

    setInMolecule(TRUE);

    di.doColorConvert = TRUE;

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
    converter = 
    ((XilDeviceManagerCompressionH261*)getDeviceManager())->getColorConverter();
    if(converter == NULL) {
        return XIL_FAILURE;
    }
    //
    // Set the generic object pointer in DecompressInfo so
    // that this object can be passed around. 
    // (Sort of a hack, but it works).
    //
    di.objectPtr1 = converter;

    //
    // Seek to the proper frame. Deferred execution may have
    // caused other frames to be decompressed before this one.
    //
    seekFlush(di.frame_number);

    //
    // Decoder object to hold info specific to this frame
    //
    H261DecompressorData* decoder = getH261DecompressorData();

    //
    // Read the bitstream header
    //
    if (!decoder->setByteStreamPtr()) {
      //
      // Decompress: no frame from buffer mgr
      // Internal error.
      //
      XIL_ERROR(system_state, XIL_ERROR_SYSTEM,"di-95",FALSE);
      return XIL_FAILURE;
    }

    //
    // Decompress the frame and copy it to the dst image
    // The copy is from the internal image buffer of the decoder.
    //
    decoder->color_video_frame(&di);
    decoder->finishDecode();

    return XIL_SUCCESS; 
}

XilStatus
XilDeviceCompressionH261::decompressOrderedDither(XilOp*       op,
                                                  unsigned int op_count,
                                                  XilRoi*      roi,
                                                  XilBoxList*  bl)
{
    float scale[3]  = {1.0F, 1.0F, 1.0F };
    float offset[3] = {0.0F, 0.0F, 0.0F };

    //
    // Construct the utility object to get all
    // size and storage data
    //
    DecompressInfo di(op, op_count, roi, bl);
    if(! di.isOK()) {
        return XIL_FAILURE;
    }

    setInMolecule(TRUE);

    di.doOrderedDither = TRUE;

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

    if(! validDecompressOrderedDither(cube, dmask)) {
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
    // This will initialize the tables which speed ordered dither
    //
    ditherTable = getDitherTable(cube, dmask, scale, offset);
    if(ditherTable == NULL) {
        return XIL_FAILURE;
    }

    //
    // Set the generic object pointer in DecompressInfo so
    // that this object can be passed around. 
    // (Sort of a hack, but it works).
    //
    di.objectPtr1 = ditherTable;

    //
    // Seek to the proper frame. Deferred execution may have
    // caused other frames to be decompressed before this one.
    //
    seekFlush(di.frame_number);

    //
    // Decoder object to hold info specific to this frame
    //
    H261DecompressorData* decoder = getH261DecompressorData();

    //
    // Read the bitstream header
    //
    if (!decoder->setByteStreamPtr()) {
      //
      // Decompress: no frame from buffer mgr
      // Internal error.
      //
      XIL_ERROR(system_state, XIL_ERROR_SYSTEM,"di-95",FALSE);
      return XIL_FAILURE;
    }

    //
    // Decompress the frame and copy it to the dst image
    // The copy is from the internal image buffer of the decoder.
    //
    decoder->color_video_frame(&di);
    decoder->finishDecode();

    return XIL_SUCCESS; 
}

//
// Verify that the constraints for decompressColorConvert Molecules
// are satisfied (ycc601->rgb709 color cvt)
//
Xil_boolean
XilDeviceCompressionH261::validDecompressColorConvert(XilImage* src,
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
XilDeviceCompressionH261::validDecompressOrderedDither(
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
