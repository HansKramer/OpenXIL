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
//  File:       CompressInfo.cc
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:16:24, 03/10/00
//
//  Description:
//
//    Utility object to gather information about this
//    frame compression in a single container.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)CompressInfo.cc	1.7\t00/03/10  "

#include "CompressInfo.hh"
#include "XiliUtils.hh"

//
// Constructor - Initialize all parameters
//
CompressInfo::CompressInfo(XilOp*       op,
                           unsigned int op_count,
                           XilRoi*      ,
                           XilBoxList*  bl)
 : image_storage(op_count > 1 ?
                  (op->getOpList())[op_count-1]->getSrcImage(1) :
                  op->getSrcImage(1) )
{
    isOKFlag = FALSE;

    //
    // This may be a molecule, so get the src and dst
    // from the first and last ops on the op list
    //
    XilOp** op_list = op->getOpList();
    image           = op_list[op_count-1]->getSrcImage();
    cis             = op_list[0]->getDstCis();
    system_state    = cis->getSystemState();

    //
    // Get the frame number. Deferred execution may
    // result in out of order compressions.
    //
    op_list[0]->getParam(1, &frame_number);

    image->getInfo(&image_width, &image_height, 
                   &image_nbands, &image_datatype);

    if(image_datatype == XIL_BIT) {
        image_storage_type = XIL_BAND_SEQUENTIAL;
    } else {
        image_storage_type = XIL_PIXEL_SEQUENTIAL;
    }

    //
    // Get the rectangle of the box of the src image to be compressed.
    // The op creates only a single box for compress operations.
    //
    XilBox* src_box;
    XilBox* dst_box;
    bl->getNext(&src_box, &dst_box);
    src_box->getAsRect(&image_box_x, &image_box_y, 
                       &image_box_width, &image_box_height);

    //
    // Get the storage description for the src image
    //
    // TODO: Extend this to tiled storage and XIL_GENERAL storage
    //
    if(image->getStorage(&image_storage, op, src_box, "XilMemory",
                         XIL_READ_ONLY, image_storage_type) == XIL_FAILURE) {
        bl->markAsFailed();
        return;
    }
    image_storage.getStorageInfo(&image_ps, &image_ss, &image_bs, 
                                 &image_box_offset, &image_box_dataptr);
    image_offset  = image_box_offset;
    image_dataptr = image_box_dataptr;

    //
    // Set the amount of data in image
    // (used for buffer size calculations)
    // 
    if(image_datatype == XIL_BIT) {
        unsigned int stride = (image_box_width + XIL_BIT_ALIGNMENT-1) /
                              XIL_BIT_ALIGNMENT;
        stride *= (XIL_BIT_ALIGNMENT/8);
        image_data_quantity = stride * image_box_height * image_nbands;
    } else {
        image_data_quantity = image_box_width * image_box_height *
                              image_nbands * xili_sizeof(image_datatype);
    }


    //
    // Get the input type of the CIS. Once a frame has been
    // compressed, the input_type is enforced for future frames
    // unless a reset is done.
    //
    XilImageFormat* cis_input_type = cis->getDeviceCompression()->getInputType();
    cis_input_type->getInfo(&cis_width, &cis_height, 
                            &cis_nbands, &cis_datatype);
    cis_box_width  = cis_width;
    cis_box_height = cis_height;
    cis_dataptr = cis_box_dataptr = NULL;
    
    isOKFlag = TRUE;
}

CompressInfo::~CompressInfo()
{
}

Xil_boolean
CompressInfo::isOK()
{
    if(this == NULL) {
        return FALSE;
    } else {
        if(isOKFlag == TRUE) {
            return TRUE;
        } else {
            delete this;
            return FALSE;
        }
    }
}

