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
//  File:       DecompressInfo.hh
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
#pragma ident   "@(#)DecompressInfo.hh	1.6\t00/03/10  "

#ifndef _DECOMPRESS_INFO_HH_
#define _DECOMPRESS_INFO_HH_

#include <xil/xilGPI.hh>

class DecompressInfo {
public:
    //
    // Constructor/ Destructor
    //
                    //
                    // Normal constructor
                    //
                    DecompressInfo(XilOp*       op,
                                  unsigned int op_count,
                                  XilRoi*      roi,
                                  XilBoxList*  box);
                   
                    //
                    // Constructor for temporary buffers,
                    // used as a temporary dst image when the final
                    // dst image has multiple rectangles (from ROIs)
                    // Takes orig DecompressInfo as its argument.
                    //
                    DecompressInfo(DecompressInfo* di);

                    //
                    // Constructor which accepts a storage description
                    //
                    DecompressInfo(DecompressInfo* di,
                                   unsigned int width,
                                   unsigned int height,
                                   unsigned int nbands,
                                   XilDataType  datatype,
                                   unsigned int ps,
                                   unsigned int ss,
                                   void*        dataptr);

                    ~DecompressInfo();

    //
    // Check that construction succeded
    //
    Xil_boolean     isOK();

    //
    // Utility method to copy from a decompresed image
    // in a temporarry buffer, to a complex destination,
    // i.e. one with multiple rectangles, due to a ROI
    //
    void            copyRects(DecompressInfo* di);

    //
    // Public destination image parameters
    //
    unsigned int    image_width;
    unsigned int    image_height;
    unsigned int    image_nbands;
    XilDataType     image_datatype;
    XilStorageType  image_storage_type;

    //
    // PIXEL_SEQUENTIAL storage params
    //
    unsigned int    image_ps;
    unsigned int    image_ss;
    unsigned int    image_bs;
    unsigned int    image_offset;
    void*           image_dataptr;

    //
    // Number of bytes to hold complete decompressed image
    // (needed for temporary buffer allocation)
    //
    unsigned int    image_data_quantity;


    //
    // GENERAL storage params
    //
    unsigned int*   image_ps_array;
    unsigned int*   image_ss_array;
    unsigned int*   image_offset_array;
    void**          image_dataptr_array;

    XilBox*         image_box;
    int             image_box_x;
    int             image_box_y;
    unsigned int    image_box_width;
    unsigned int    image_box_height;
    unsigned int    image_box_offset;
    void*           image_box_dataptr;

    //
    // Public source cis parameters
    //
    unsigned int    cis_width;
    unsigned int    cis_height;
    unsigned int    cis_nbands;
    XilDataType     cis_datatype;

    XilBox*         cis_box;
    int             cis_box_x;
    int             cis_box_y;
    unsigned int    cis_box_width;
    unsigned int    cis_box_height;
    void*           cis_box_dataptr;

    XilImage*       image;
    XilCis*         cis;
    int             frame_number;
    XilRoi*         image_roi;
    XilBoxList*     image_bl;

    XilSystemState* system_state;

    //
    // Booleans to support the common molecules
    //
    Xil_boolean     doColorConvert;
    Xil_boolean     doOrderedDither;
    Xil_boolean     doScale;

    //
    // Generic pointers to pass around
    // objects such as those used for color conversion
    // and dithering.
    //
    void*           objectPtr1;
    void*           objectPtr2;
    void*           objectPtr3;

private:
    XilStorage*     image_storage;

    //
    // Flag to identify that this object holds temporary storage
    // for use in dealing with ROIs on the destination image
    //
    Xil_boolean     mustDeleteTemporary;

    Xil_boolean     isOKFlag;
};

#endif // _DECOMPRESS_INFO_HH_
