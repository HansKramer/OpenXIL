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
//  File:       Mpeg1Molecules.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:14:47, 03/10/00
//
//  Description:
//
//    Support for Mpeg1 Molecules
//        decompress:ordered_dither:
//        decompress:rescale:ordered_dither
//        decompress:color_convert
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Mpeg1Molecules.cc	1.2\t00/03/10  "

#include <stdio.h>
#include <xil/xilGPI.hh>
#include "XilDeviceCompressionMpeg1.hh"

//------------------------------------------------------------------------
//
//  Function:	DecompressDither8
//
//  Description:
//	8 bit Ordered Dither molecule
//
//  Returns:
//	XIL_SUCCESS if molecule is successful
//	XIL_FAILURE if molecule cannot be used (and we should try atomic path)
//	
//	
//------------------------------------------------------------------------

//
// Molecule definitions for Mpeg1 dither-to-memory
//

/* XILCONFIG: DecompressDither8= ordereddither8_8(decompress_Mpeg1()) */
/* XILCONFIG: DecompressDither8= ordereddither8_8(rescale8(decompress_Mpeg1())) */

XilStatus
XilDeviceCompressionMpeg1::DecompressDither8(XilOp*       op,
                                             unsigned int op_count,
                                             XilRoi*      roi,
                                             XilBoxList*  bl)
{

    float*                     rescale;
    float*                     offset;
    static                     rescale8_op = -1;

    //
    //  Pull everything we need off of the DAG.
    //
    //  First, the ordered dither op...
    //
    XilImage*     dst   = op->getDst();
    XilLookup*    cube  = (XilLookup *)op->getObjParam(1);  
    XilDithermask dmask = (XilDitherMask *)op->getObjParam(2);
    NEXTOP(op, op_count);

    //
    //  Now, the optional rescale op...
    //
    GETOPNUM(rescale8_op, "rescale8");
    if (op->getOp() == rescale8_op) {
      rescale = (float *)op->getPtrParam(1);  
      offset  = (float *)op->getPtrParam(2);  
      NEXTOP(op,op_count);
    } else {
      rescale = 0;
      offset  = 0;
    }

    //
    //  Now, the decompress op...
    //
    setInMolecule(TRUE);
    seek((int)op->getLongParam(1));
    Mpeg1DecompressorData* decoder = getMpeg1DecompressorData();
    
    unsigned int width = (unsigned short)decoder->bitstreamWidth;
    unsigned int height = (unsigned short)decoder->bitstreamHeight;

    //
    // Generic tests for molecule compatibility
    //
    if (!(is_vanilla_image(dst, width, height)))
        return XIL_FAILURE;
    //
    // width and height must be a multiple of 16
    //  
    if ((width & 0x0f) || (height & 0x0f))
        return XIL_FAILURE; 

    //
    // Set imager state to relevant dither parameters
    //
    if (!decoder->imager.validateOrdDith8(dst, 0,
                        (int) width,
                        (int) height,
                        cube, dmask, rescale, offset, -128))
	return XIL_FAILURE;

    // 
    // check history buffers in case skipped frames, setup at proper read position
    //
    int status = decoder->checkHistoryBuffers();


    decoder->setDitheringFlag(1);
    if (status == 0)
      decoder->outputLastBuffer();
    else {
      if ( !decoder->setByteStreamPtr()) {

         // We have a condition where 
         //   decompress: no frame from buffer mgr
         // However, we are reporting: Mpeg1 bytestream error.
         // (should we tell users more?)

         XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-98", TRUE);
         return XIL_FAILURE;
       }

       decoder->color_video_frame();
       decoder->finishDecode();
    }
    return XIL_SUCCESS;
    

}
//------------------------------------------------------------------------
//
//  Function:	decompressColorConvert
//
//  Description:
//	Decompress/colorconvert (YCC to RGB) a frame to memory.
//
//  Parameters:
//    XilOp*     op,           // a pointer into the DAG
//    int op_count)            // the number of combined ops to be done
//	
//  Returns:
//	XIL_SUCCESS if molecule is successful
//	XIL_FAILURE if molecule cannot be used (and we should try atomic path)
//	
//  Deficiencies/ToDo:
//
//------------------------------------------------------------------------

/* XILCONFIG: decompressColorConvert= colorconvert(decompress_Mpeg1())  */

XilStatus     
XilDeviceCompressionMpeg1::decompressColorConvert(XilOp*       op,
                                                  unsigned int op_count,
                                                  XilRoi*      roi,
                                                  XilBoxList*  bl)
{

    XilImage* src_color_convert;
    XilImage* dst_color_convert;
    Mpeg1DecompressorData* decoder;

    //
    //  Pull everything we need off of the DAG.
    //
    //  First, the (optional) scale  op...
    //
    XilImage*     dst = op->getDst();

    //
    //  Now, the color convert op...
    //
    src_color_convert = op->getSrc1();
    dst_color_convert = op->getDst();
    NEXTOP(op, op_count);

    //
    //  Now, the decompress op...
    //
    setInMolecule(TRUE);
    seek((int)op->getLongParam(1));
    decoder = getMpeg1DecompressorData();

    unsigned int width =  decoder->bitstreamWidth;
    unsigned int height = decoder->bitstreamHeight;

    //
    // Generic tests for molecule compatibility
    //
    if (!(is_vanilla_image(dst, width, height)))
        return XIL_FAILURE;

    //
    // width and height must be a multiple of 16
    //  
    if ((width & 0x0f) || (height & 0x0f))
        return XIL_FAILURE; 

    //
    // Set imager state to relevant parameters
    // This will also ensure that the parameters are acceptable for this
    // molecule, e.g. src image is YCC.
    //
    if (!decoder->imager.validateColorConvert(dst,
			src_color_convert, 
			dst_color_convert,
			0,
                        (int) width,
                        (int) height,
			-128))
        return XIL_FAILURE;

    //
    // Perform color conversion into the destination image, dst.
    //

    // 
    // Check history buffers in case skipped frames, 
    // setup at proper read position
    //
    int status = decoder->checkHistoryBuffers();


    decoder->setDitheringFlag(1);
    if (status == 0)
      decoder->outputLastBuffer();
    else {
      if ( !decoder->setByteStreamPtr()) {

         //
         // We have a condition where 
         // decompress: no frame from buffer mgr
         // However, we are reporting: Mpeg1 bytestream error.
         // (should we tell users more?)
         //
         XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-98", TRUE);
         return XIL_FAILURE;
       }

       decoder->color_video_frame();
       decoder->finishDecode();
    }
    return XIL_SUCCESS;

}

