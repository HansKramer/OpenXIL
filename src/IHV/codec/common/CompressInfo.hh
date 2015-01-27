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
//  File:       CompressInfo.hh
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:23:53, 03/10/00
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
#pragma ident   "@(#)CompressInfo.hh	1.6\t00/03/10  "

#ifndef _COMPRESS_INFO_HH_
#define _COMPRESS_INFO_HH_

#include <xil/xilGPI.hh>

class CompressInfo {
public:
    //
    // Public member functions
    //
                   CompressInfo(XilOp*       op,
                                unsigned int op_count,
                                XilRoi*      roi,
                                XilBoxList*  box);

                   ~CompressInfo();

    Xil_boolean    isOK();

    //
    // Public source image parameters
    //
    unsigned int   image_width;
    unsigned int   image_height;
    unsigned int   image_nbands;
    XilDataType    image_datatype;
    XilStorageType image_storage_type;

    //
    // PIXEL_SEQUENTIAL storage params
    //
    unsigned int   image_ps;
    unsigned int   image_ss;
    unsigned int   image_bs;
    unsigned int   image_offset;
    void*          image_dataptr;

    //
    // Number of bytes to hold image
    // (used for buffer allocation)
    //
    unsigned int   image_data_quantity;

    //
    // GENERAL storage params
    //
    unsigned int*  image_ps_array;
    unsigned int*  image_ss_array;
    unsigned int*  image_offset_array;
    void**         image_dataptr_array;

    int            image_box_x;
    int            image_box_y;
    unsigned int   image_box_width;
    unsigned int   image_box_height;
    unsigned int   image_box_offset;
    void*          image_box_dataptr;

    //
    // Public destination cis parameters
    //
    unsigned int   cis_width;
    unsigned int   cis_height;
    unsigned int   cis_nbands;
    XilDataType    cis_datatype;
    void*          cis_dataptr;
    int            cis_box_x;
    int            cis_box_y;
    unsigned int   cis_box_width;
    unsigned int   cis_box_height;
    void*          cis_box_dataptr;

    XilImage*      image;
    XilCis*        cis;
    int            frame_number;

    XilSystemState* system_state;

private:
    XilStorage     image_storage;
    Xil_boolean    isOKFlag;

};

#endif // _COMPRESS_INFO_HH_
