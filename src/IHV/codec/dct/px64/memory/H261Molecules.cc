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
//  File:	H261Molecules.cc
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	16:10:43, 31 Jan 1994
//
//  Description:
//	
//	
//	
//	
//	
//------------------------------------------------------------------------
//
//	Copyright (c) 1992, 1993, 1994, by Sun Microsystems, Inc.
//
//------------------------------------------------------------------------

#pragma ident	"@(#)H261Molecules.cc	1.5\t94/01/31  "


#include <stdio.h>
#include <xil/xilGPI.hh>
#include "H261DecompressorData.h"
#include "IdctImager.hh"
#include "XilDeviceCompressionH261.hh"

//------------------------------------------------------------------------
//
//  Function:	DecompressDither8
//  Created:	01/01/92
//
//  Description:
//	8 bit Ordered Dither molecule
//
//  Parameters:
//    XilOp*     op,           // a pointer into the DAG
//    int op_count)            // the number of combined ops to be done
//	
//  Returns:
//	XIL_SUCCESS if molecule is successful
//	XIL_FAILURE if molecule cannot be used (and we should try atomic path)
//	
//	
//------------------------------------------------------------------------

//
// molecule definitions for H261 dither-to-memory
//

/* XILCONFIG: DecompressDither8= ordereddither8_8(decompress_H261()) */
/* XILCONFIG: DecompressDither8= ordereddither8_8(rescale8(decompress_H261())) */

XilStatus
XilDeviceCompressionH261::DecompressDither8(
    XilOp*      op,             // a pointer into the DAG
    int         op_count)       // the number of combined ops to be done
{

    XilLookup*      cube;
    XilDitherMask*  dmask;
    XilDeviceCompressionH261* dc;
    static rescale8_op = -1;
    float* rescale;
    float* offset;
    H261DecompressorData* decoder;

    //
    //  Pull everything we need off of the DAG.
    //
    //  First, the ordered dither op...
    //
    XilImage*     dst = op->getDst();
    cube = (XilLookup *)op->getObjParam(1);  
    dmask = (XilDitherMask *)op->getObjParam(2);
    NEXTOP(op, op_count);

    //
    //  Now, the optional rescale op...
    //
    GETOPNUM(rescale8_op, "rescale8");
    if (op->getOp() == rescale8_op) {
      rescale = (float *)op->getPtrParam(1);  
      offset = (float *)op->getPtrParam(2);  
      NEXTOP(op,op_count);
    } else {
      rescale = 0;
      offset = 0;
    }

    //
    //  Now, the decompress op...
    //
    dc = (XilDeviceCompressionH261*)
	( op->getSrcCis() )->getDeviceCompression();
    dc->setInMolecule(TRUE);
    dc->seekFlush((int)op->getLongParam(1));
    decoder = dc->getH261DecompressorData();
    
    if ( !decoder->setByteStreamPtr()) {

      // We have a condition where 
      //   decompress: no frame from buffer mgr
      // However, we are reporting: H261 bytestream error.
      // (should we tell users more?)

      XIL_ERROR(system_state, XIL_ERROR_SYSTEM, "di-98", TRUE);
      return XIL_FAILURE;
    }

    unsigned short width = (unsigned short)decoder->bitstreamWidth;
    unsigned short height = (unsigned short)decoder->bitstreamHeight;

    /*
     * Generic tests for molecule compatibility
     */
    if (!(is_vanilla_image(dst, width, height)))
        return XIL_FAILURE;
    
    /*
     * width and height must be a multiple of 16
     * In H.261 we know that they always are.  (MPEG1 does not guarantee this)
     */
    /*
     * Set imager state to relevant dither parameters
     */
    if (!decoder->imager.validateOrdDith8(dst, 0,
                        (int) width,
                        (int) height,
                        cube, dmask, rescale, offset, -128))
	return XIL_FAILURE;

    decoder->setDitheringFlag(1);
    decoder->color_video_frame(&decoder->imager);
 
    decoder->finishDecode();

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

/* XILCONFIG: decompressColorConvert= colorconvert(decompress_H261())  */
XilStatus 
XilDeviceCompressionH261::decompressColorConvert(
    XilOp*     op,           // a pointer into the DAG
    int op_count)            // the number of combined ops to be done
{

    XilDeviceCompressionH261* dc;
    XilImage* src_color_convert;
    XilImage* dst_color_convert;
    H261DecompressorData* decoder;

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
    dc = (XilDeviceCompressionH261*)
	( op->getSrcCis() )->getDeviceCompression();
    dc->setInMolecule(TRUE);
    dc->seekFlush((int)op->getLongParam(1));
    decoder = dc->getH261DecompressorData();

    if ( !decoder->setByteStreamPtr()) {
 
      // We have a condition where
      //   decompress: no frame from buffer mgr
      // However, we are reporting: H261 bytestream error.
      // (should we tell users more?)
 
      XIL_ERROR(system_state, XIL_ERROR_SYSTEM, "di-98", TRUE);
      return XIL_FAILURE;
    }

    unsigned short width = (unsigned short)decoder->bitstreamWidth;
    unsigned short height = (unsigned short)decoder->bitstreamHeight;

    /*
     * Generic tests for molecule compatibility
     */
    if (!(is_vanilla_image(dst, width, height)))
        return XIL_FAILURE;

    /*
     * width and height must be a multiple of 16
     * In H.261 we know that they always are.  (MPEG1 does not guarantee this)
     */

    /*
     * Set imager state to relevant parameters
     * This will also ensure that the parameters are acceptable for this
     * molecule, e.g. src image is YCC.
     */
    if (!decoder->imager.validateColorConvert(dst,
			src_color_convert, 
			dst_color_convert,
			0,
                        (int) width,
                        (int) height,
			-128))
        return XIL_FAILURE;

    /*
     * Perform color conversion into the destination image, dst.
     */
    decoder->setDitheringFlag(1);
    decoder->color_video_frame(&decoder->imager);
 
    decoder->finishDecode();
 
    return XIL_SUCCESS;

}
