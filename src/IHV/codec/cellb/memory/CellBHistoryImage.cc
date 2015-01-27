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
//  File:   CellBHistoryImage.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:15:27, 03/10/00
//
//  Description:
//
//
//
//
//
//
//
//
//  MT-level:  <??????>
//
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)CellBHistoryImage.cc	1.3\t00/03/10  "

#include <xil/xilGPI.hh>

#include "CellBHistoryImage.hh"

CellBHistoryImage::~CellBHistoryImage()
{
    image->destroy();
    parent->destroy();
}


#ifdef MOLECULE_SUPPORT

CellBHistoryImage::CellBHistoryImage(unsigned int w,
                                     unsigned int h,
                                     unsigned int f,
                                     unsigned int num_bands, 
                                     unsigned int parent_num_bands,
                                     unsigned int b_offset)
{
    isOKFlag = FALSE;

    //
    //  Ensure these are NULL for destructor.
    //
    image  = NULL;
    parent = NULL;

    //
    //  Create a image of the width & height of the CIS
    //
    image = system_state->createXilImage(w, h, parent_num_bands, XIL_BYTE);
    if(image == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", FALSE);
        return;
    }
    image->export();

    if(parent_num_bands == num_bands) {
        parent = NULL;
    } else {
        parent = image;

        image = parent->createChild(0,0,w,h,b_offset,num_bands);
        if(image == NULL) {
            XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", FALSE);
            parent->destroy();
            return;
        }
        image->export();
    }

    valid = FALSE;
    frame_no = f;
    width = w;
    height = h;
    this->nbands = num_bands;
    this->parent_bands = parent_num_bands;
    this->band_offset = b_offset;

    isOKFlag = TRUE;
}

#endif

Xil_boolean
CellBHistoryImage::verifyImage(unsigned int w,
                               unsigned int h,
                               unsigned int num_bands,
                               unsigned int parent_num_bands, 
                               unsigned int b_offset)
{
    return (w == width) &&
           (h == height) &&
           (this->nbands == num_bands) &&
           (this->parent_bands == parent_num_bands) &&
           (this->band_offset == b_offset);
}


