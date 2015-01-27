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
//  File:	decompress.cc
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:14:07, 03/10/00
//
//  Description:
//	The decompress function for the Identity codec
//	
//	
//------------------------------------------------------------------------
#pragma ident	"@(#)decompress.cc	1.7\t00/03/10  "

#include "XilDeviceCompressionIdentity.hh"

#define IDENTITY_BYTESTREAM_ERROR(bp, ftype) \
{ \
   cbm->decompressedFrame((Xil_unsigned8*)bp, ftype); \
   XIL_CIS_ERROR(XIL_ERROR_SYSTEM, "di-285", TRUE, this, FALSE, FALSE); \
   return XIL_FAILURE; \
}

XilStatus
XilDeviceCompressionIdentity::decompress(XilOp*       op, 
                                         unsigned int , 
                                         XilRoi*      roi, 
                                         XilBoxList*  bl)
{
    //
    // Split the box list on tile boundaries.
    // Currently this is a no-op for compression.
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Get the cis and destination image off of the DAG
    //
    XilImage* dst_image = op->getDstImage(1);
    XilCis*   src_cis   = op->getSrcCis(1);

    //
    //  The frame number which we're supposed to decompress is
    //  specified by the first parameter on the Op.  So, we'll seek to
    //  that frame.
    //
    int frame_number;
    op->getParam(1, &frame_number);
    seek(frame_number);

    // In order to illustrate "key" frames, 
    // this codec marks even frames with its own frame type
    // This illustrates the use of frame type with the
    // compressedFrame/decompressFrame/seek/adjustStart routines
    // (Of course, codecs generally have a much better reason
    // to mark a frame as a "key" frame!)

    // Test odd/even frame for decompress
    int frame_type;
    if(frame_number & 0x1) {
      // odd frame, no special frame type
      frame_type = XIL_CIS_DEFAULT_FRAME_TYPE;
    } else {
      // even frame, mark it as our key frame
      frame_type = IDENTITY_FRAME_TYPE;
    }

    //
    //  Get the information about the CIS image type.
    //
    XilImageFormat*  cis_outtype = this->getOutputType();

    unsigned int cis_width;
    unsigned int cis_height;
    unsigned int cis_bands;
    XilDataType datatype;
    cis_outtype->getInfo(&cis_width, &cis_height, &cis_bands, &datatype);

    //
    //  Get the pointer to the data to decompress...
    //
    Xil_unsigned32* bp32 = (Xil_unsigned32*) cbm->nextFrame();
    if(bp32 == NULL) {
	// XilCis: No data to decompress
        XIL_CIS_ERROR(XIL_ERROR_SYSTEM, "di-100", TRUE, this, FALSE, FALSE);
	return XIL_FAILURE;
    }
    
    //
    //  Just in case we've had an error before, we don't want to
    //  SEGV trying to access a word when non-word aligned
    //
    if((unsigned int)bp32 % sizeof(unsigned int)) {
        IDENTITY_BYTESTREAM_ERROR(bp32, frame_type);
    }
    if(*bp32++ != cis_width) {
        IDENTITY_BYTESTREAM_ERROR(bp32, frame_type);
    }
    if(*bp32++ != cis_height) {
        IDENTITY_BYTESTREAM_ERROR(bp32, frame_type);
    }
    if(*bp32++ != cis_bands) {
        IDENTITY_BYTESTREAM_ERROR(bp32, frame_type);
    }

    Xil_unsigned8* bp = (Xil_unsigned8*) bp32;

    XilBox*      image_box;
    XilBox*      cis_box;
    unsigned int boxcount = 0;

    //
    // Process each box. Actually the core delivers only
    // a single src and dst box, but we use the normal box
    // processing loop for consistency. Note that even though
    // there is a single destination box, it can have multiple
    // rectangles within it due to the presence of ROIs.
    //
    while(bl->getNext(&cis_box, &image_box)) {
        XilStorage storage(dst_image);


        //
        // Request the storage as XIL_PIXEL_SEQUENTIAL. 
        // This will force conversion if the storage is some
        // other type, so this example is not a "complete" implementation.
        //
        if(dst_image->getStorage(&storage, op, image_box, "XilMemory", 
                                 XIL_READ_ONLY, 
                                 XIL_PIXEL_SEQUENTIAL) == XIL_FAILURE) {
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

        unsigned int   ps;
        unsigned int   ss;
        Xil_unsigned8* p_data;
        storage.getStorageInfo(&ps, &ss, NULL, NULL, (void**)&p_data);

        //
        // Create a list of dst image rectangles to process for this box.
        // The identity decompress function really only works if there
        // is a single rectangle, but in general, we want to enable
        // the decompressor to function with arbitrary dst regions
        //
        XilRectList rl(roi, image_box);

        int          x,y;
        unsigned int w, h;
        while(rl.getNext(&x, &y, &w, &h)) {

            Xil_unsigned8* p_scan = p_data;
            for(int i=0; i<cis_height; i++) {
                Xil_unsigned8* p_pixel = p_scan;
                for(int j=0; j<cis_width; j++) {
                    Xil_unsigned8* pband = p_pixel;
                    for(int k=0; k<cis_bands; k++) {
                        *pband++ = *bp++;
                    }
                    p_pixel += ps;
                }
                p_scan += ss;
            }

        }

        boxcount++;
    }
        
    cbm->decompressedFrame(bp, frame_type);
    
    return XIL_SUCCESS;
}
