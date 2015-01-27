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
//  File:	XiliRoiBitmask.cc
//  Project:	XIL
//  Revision:	1.22
//  Last Mod:	10:08:29, 03/10/00
//
//  Description: This file is the Bitmask implementation of the ROI.
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
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliRoiBitmask.cc	1.22\t00/03/10  "

#include "_XilDefines.h"
#include "_XilRoi.hh"
#include "_XilBox.hh"
#include "_XilImage.hh"

XiliRoiBitmask::XiliRoiBitmask(XilRoi* calling_roi) :
    bbox(0, 0, 0, 0)
{
    bitmask = NULL;
    myRoi   = calling_roi;
    valid   = FALSE;
    
}

XiliRoiBitmask::~XiliRoiBitmask()
{
    bitmask->destroy();
}

XilStatus
XiliRoiBitmask::clear()
{
    bitmask->destroy();

    bbox.set(0, 0, 0, 0);

    valid = FALSE;

    return XIL_SUCCESS;
}

const XiliRoiBitmask&
XiliRoiBitmask::operator =(XiliRoiBitmask& from)
{
    //
    // TODO: maynard 2/16/95
    //       run through data setting equal.
    //
    myRoi = from.myRoi;
    valid = from.valid;

    //
    // set the bounding box
    //
    bbox  = from.bbox;

    return *this;
}

void
XiliRoiBitmask::setCallingRoi(XilRoi* top_roi)
{
    myRoi = top_roi;
}

void
XiliRoiBitmask::setValid(Xil_boolean valid_flag)
{
    valid = valid_flag;
}

Xil_boolean
XiliRoiBitmask::isValid() const
{
    return valid;
}

XiliRect*
XiliRoiBitmask::getBbox()
{
    return &bbox;
}

XilImage*
XiliRoiBitmask::getBitmask()
{
    return bitmask;
}

XilStatus
XiliRoiBitmask::addRect(int /*x*/,
			int /*y*/,
			unsigned int /*width*/,
			unsigned int /*height*/)
{
    return XIL_FAILURE;
}

XilStatus
XiliRoiBitmask::subtractRect(int /*x*/,
			     int /*y*/,
			     unsigned int /*width*/,
			     unsigned int /*height*/)
{
    return XIL_FAILURE;
}

XilStatus
XiliRoiBitmask::addImage(XilImage* /*image*/)
{
    return XIL_FAILURE;
}

XilImage*
XiliRoiBitmask::getAsImage()
{
    return NULL;
}

XilStatus
XiliRoiBitmask::intBoundingBox(int* x,
                               int* y,
                               unsigned int* xsize,
                               unsigned int* ysize)
{
    int x2;
    int y2;

    bbox.get(x, y, &x2, &y2);
    *xsize = x2-*x+1;
    *ysize = y2-*y+1;
    return XIL_SUCCESS;
}

XilStatus
XiliRoiBitmask::getCopyRoiBitmask(XiliRoiBitmask* /*copy*/)
{
    return XIL_FAILURE;
}

XilStatus
XiliRoiBitmask::intersect(const XiliRoiBitmask* /*other_bm*/,XiliRoiBitmask* /*intersection*/)
{
    return XIL_FAILURE;
}

XilStatus
XiliRoiBitmask::unite(const XiliRoiBitmask* /*other_bm*/, XiliRoiBitmask* /*unite_bm*/)
{
    return XIL_FAILURE;
}

XilStatus
XiliRoiBitmask::translateRectList(XiliRoiRect* rl)
{
    if(rl == NULL) {
        return XIL_FAILURE;
    }

    //
    // Empty out existing bitmask data for simplicity
    //
    this->clear();
    if(rl->numRects() == 0) {
        return XIL_SUCCESS;
    }

    int          boxx;
    int          boxy;
    unsigned int boxxsize;
    unsigned int boxysize;

    //
    // Bounding box of the rectlist will determine the size of the bitmask image
    //
    rl->intBoundingBox(&boxx, &boxy, &boxxsize, &boxysize);

    //
    // create the image
    //
    bitmask = myRoi->getSystemState()->createXilImage(boxxsize,
                                                      boxysize,
                                                      1,
                                                      XIL_BIT);
    if(!bitmask) {
        XIL_ERROR(myRoi->getSystemState(), XIL_ERROR_SYSTEM,"di-147",FALSE);
        return XIL_FAILURE;
    }
    
    //
    // The origin will hold the roi bbox starting point
    //
    bitmask->setOrigin((float)boxx,(float)boxy);

    //
    // get the storage - cobbled together for now
    // 
    XilStorage storage(bitmask);
    XilBox imagebox(0, 0, boxxsize, boxysize);
    if(bitmask->getStorage(&storage, &imagebox,"XilMemory",
                           XIL_WRITE_ONLY,
                           XIL_STORAGE_TYPE_UNDEFINED, NULL) == XIL_FAILURE) {
        XIL_ERROR(myRoi->getSystemState(),XIL_ERROR_SYSTEM,"di-140",FALSE);
        return XIL_FAILURE;
    }


    //
    // We can assume that the image is a 1-band, BIT image and therefore, 
    // next_pixel and next_band are unused...
    //
    unsigned int next_scanline, base_offset;
    Xil_unsigned8* base_addr;
    if(storage.getStorageInfo((unsigned int*)NULL,
                              &next_scanline,
                              (unsigned int*)NULL,
                              &base_offset,
                              (void**)&base_addr) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    
    Xil_unsigned8* scanline = base_addr;
    unsigned int  offset = base_offset;
    
    //
    // clear image
    //
    for(unsigned int k = 0; k < boxysize; k++) {
        for(unsigned int m = 0; m < boxxsize; m++) {
            XIL_BMAP_CLR(scanline, offset + m);
        }
        scanline += next_scanline;
    }
    
    //
    // set the appropriate pixels in the image
    //
    XiliRectInt* current_rect;
    unsigned int width, height;
    current_rect = rl->getRectList();
    while(current_rect != NULL) {
        unsigned int xstart = (unsigned int)(current_rect->getX1() - boxx);
        unsigned int ystart = (unsigned int)(current_rect->getY1() - boxy);
        
        width =  (unsigned int)(current_rect->getX2() - current_rect->getX1() + 1);
        height = (unsigned int)(current_rect->getY2() - current_rect->getY1() + 1);
        scanline = base_addr + next_scanline * ystart;
        offset = (unsigned int)base_offset + xstart;
        
        for(unsigned int i = 0; i < height; i++) {
            for (unsigned int j = 0; j < width; j++) {
                XIL_BMAP_SET(scanline, offset + j);
            }
            scanline += next_scanline;
        }
        current_rect = (XiliRectInt*)current_rect->getNext();
    }
    bbox.set(boxx, boxy, boxx+boxxsize-1, boxy+boxysize-1);

    return XIL_SUCCESS;
}
