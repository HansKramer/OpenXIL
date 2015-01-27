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
//  File:   CellBHistoryImage.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:19, 03/10/00
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
#pragma ident   "@(#)CellBHistoryImage.hh	1.3\t00/03/10  "

#ifndef CELLBHISTORYIMAGE_HH
#define CELLBHISTORYIMAGE_HH

#include <xil/xilGPI.hh>

class CellBHistoryImage
{
public:
    Xil_boolean ok() { return isOKFlag; }

    XilImage* getImage() { return image; }

    Xil_boolean getValid() { return valid; }

    void updateImage(unsigned int f) 
    {
      valid = (f == frame_no + 1);
      frame_no = f;
    }

    Xil_boolean verifyImage(unsigned int w,
                            unsigned int h,
                            unsigned int nbands,
                            unsigned int parent_bands, 
                            unsigned int band_offset);

    CellBHistoryImage(unsigned int w,
                      unsigned int h,
                      unsigned int f,
                      unsigned int nbands,
                      unsigned int parent_bands, 
                      unsigned int band_offset);

    ~CellBHistoryImage();

private:
    Xil_boolean isOKFlag;
    XilImage*   image;
    XilImage*   parent;
    Xil_boolean valid;
    int frame_no;

    unsigned int width;
    unsigned int height;
    unsigned int nbands;
    unsigned int parent_bands;
    unsigned int band_offset;
};

#endif // CELLBHISTORYIMAGE_HH
