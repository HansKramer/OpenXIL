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
//  File:	compress.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:14:06, 03/10/00
//
//  Description:
//	The compress function for the Identity Codec
//	
//	
//	
//------------------------------------------------------------------------
#pragma ident	"@(#)compress.cc	1.8\t00/03/10  "

#include "XilDeviceCompressionIdentity.hh"

XilStatus
XilDeviceCompressionIdentity::compress(XilOp*       op, 
                                       unsigned int , 
                                       XilRoi*      , 
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
    //  Get the image and the cis off of the DAG.
    //
    XilImage* image = op->getSrcImage(1);
    XilCis* dst_cis = op->getDstCis(1);

    // In order to illustrate "key" frames, 
    // this codec marks even frames with its own frame type
    // This illustrates the use of frame type with the
    // compressedFrame/decompressFrame/seek/adjustStart routines
    // (Of course, codecs generally have a much better reason
    // to mark a frame as a "key" frame!)

    //
    // Get the frame number of the compress operation.
    // Needed so we can seek to the correct frame.
    // Deferred execution may have moved things.
    //
    int frame_type;
    int frame_number;
    op->getParam(1, &frame_number);
    if(frame_number & 0x1) {
      // odd frame, no special frame type
      frame_type = XIL_CIS_DEFAULT_FRAME_TYPE;
    } else {
      // even frame, mark it as our key frame
      frame_type = IDENTITY_FRAME_TYPE;
    }

    //
    // Seek to the proper frame
    //
    seek(frame_number);

    //
    //  Local copies of image type information.
    //  Retrieve via the image format description 
    //
    unsigned int width, height, nbands;
    XilDataType datatype;
    image->getInfo(&width, &height, &nbands, &datatype);

    //
    //  Get the next buffer to compress into.
    //
    XilCisBuffer* cb = cbm->nextBuffer();
    if(cb == NULL){
        XIL_ERROR(image->getSystemState(), XIL_ERROR_RESOURCE,"di-TODO", FALSE);
        return XIL_FAILURE;
    }

    //
    //  Write the image parameters into the byte-stream
    //
    cb->addBytes((Xil_unsigned8*)&width, sizeof(width));
    cb->addBytes((Xil_unsigned8*)&height, sizeof(height));
    cb->addBytes((Xil_unsigned8*)&nbands, sizeof(nbands));
    
    //
    //  Actually perform the compression into the CisBuffer
    //

    unsigned int boxcount = 0;

    XilBox*      image_box;
    XilBox*      cis_box;
    while(bl->getNext(&image_box, &cis_box)) {
        //
        // Create a storage object on the stack using the image
        // Request storage in XIL_PIXEL_SEQUENTIAL form. This may
        // force conversion if the storage is of another form,
        // so this is not really a "complete" implementation.
        //
        XilStorage storage(image);
        if(image->getStorage(&storage, op, image_box, "XilMemory", 
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
        // Get the size of the image box. We don't need to
        // be concerned with rectangle lists here because the
        // core has already guaranteed that there are no roi's.
        //
        int          x,y;
        unsigned int w,h;
        image_box->getAsRect(&x, &y, &w, &h);

        Xil_unsigned8* p_scan = p_data + y*ss + x*ps;
        for(int i=0; i<height; i++) {
            Xil_unsigned8* p_pixel = p_scan;
            for(int j=0; j<width; j++) {
                Xil_unsigned8* p_band = p_pixel;
                for(int k=0; k<nbands; k++) {
                    cb->addByte(*p_band);
                    p_band++;
                }
                p_pixel += ps;
            }
            p_scan += ss;
        }

        boxcount++;
    }
        
    cbm->compressedFrame(frame_type);
    
    return XIL_SUCCESS;
}
