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
//  File:	XilOpIO.cc
//  Project:	XIL
//  Revision:	1.16
//  Last Mod:	10:07:21, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilOpIO.cc	1.16\t00/03/10  "


#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpIO.hh"
#include "XiliOpUtils.hh"
#include "XilOpPoint.hh"

//------------------------------------------------------------------------
//
//  Function:	moveIntoGlobalSpace()
//
//  Description:
//	Moves the given box or ROI into global space.  By default this 
//      means asking the object to convert from object space to global
//	space.
//
//------------------------------------------------------------------------
XilStatus
XilOpIO::moveIntoGlobalSpace(XiliRect*            rect,
                             XilDeferrableObject* )
{
    return realOp->moveIntoGlobalSpace(rect, realImage);
}

XilStatus
XilOpIO::moveIntoGlobalSpace(XilRoi*              roi,
                             XilDeferrableObject* )
{
    return realOp->moveIntoGlobalSpace(roi, realImage);
}

//------------------------------------------------------------------------
//
//  Function:	moveIntoObjectSpace()
//
//  Description:
//	Moves the given box or ROI into object space.  By default this 
//      means asking the object to convert from global space to object
//	space.
//
//------------------------------------------------------------------------
XilStatus
XilOpIO::moveIntoObjectSpace(XiliRect*            rect,
                             XilDeferrableObject* )
{
    return realOp->moveIntoObjectSpace(rect, realImage);
}

XilStatus
XilOpIO::moveIntoObjectSpace(XilRoi*              roi,
                             XilDeferrableObject* )
{
    return realOp->moveIntoObjectSpace(roi, realImage);
}

XilStatus
XilOpIO::clipToTile(XilDeferrableObject* ,
                    XilTileNumber        tile_number,
                    XiliRect*            rect)
{
    return realOp->clipToTile(realImage, tile_number, rect);
}

XilStatus
XilOpIO::readjustBoxStorage(XiliRect* dst_rect)
{
    if(translateX != 0 || translateY != 0) {
        dst_rect->translate(-translateX, -translateY);
    }

    return XIL_SUCCESS;
}

XilStatus
XilOpIO::switchToAlternateOp()
{
#ifdef _WINDOWS
    return XIL_FAILURE;
#else
    //
    //  If we've already mutated to the default device, then go no further.
    //
    if(mutatedToDefaultFlag) {
        return XIL_FAILURE;
    }

    //
    //  This is implemented in order to support a specific I/O framebuffer
    //  device display or capture routine failing and then permitting us to
    //  fall back to performing the operation through xlib.
    //
    //  This routine is called irrespective of whether it's a framebuffer
    //  device -- so we check a flag on the device to see whether it fits the
    //  critera. 
    //
    XilImage* img = getSrcImage(1);

    XilDeviceIO* devio = img->getDeviceIO();
    if(devio == NULL) {
        return XIL_FAILURE;
    }

    if(! devio->isFramebufferDevice()) {
        return XIL_FAILURE;
    }

    //
    //  If the image has an alternate image, then we can just switch to that.
    //  Otherwise, we need to create a new default display device and image.
    //
    XilImage* alt_img = img->getAlternateImage();
    if(alt_img == NULL) {
        XilSystemState* state = img->getSystemState();

        Display* display;
        Window   window;
        if(devio->getAttribute("WINDOW", (void**)&window) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        if(devio->getAttribute("DISPLAY", (void**)&display) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        alt_img = state->createXilImage(display, window,
                                        _XILI_DEFAULT_IO_DISPLAY);
        if(alt_img == NULL) {
            return XIL_FAILURE;
        }

        //
        //  The two images must have the same format.
        //
        if(! ((*(XilImageFormat*)alt_img) == (*(XilImageFormat*)img))) {
            alt_img->destroy();
            return XIL_FAILURE;
        }

        //
        //  After it's been set on the image, the img will take care of
        //  destroying the alt_img.  We don't need to take care of it here.
        //
        if(img->setAlternateImage(alt_img) == XIL_FAILURE) {
            return XIL_FAILURE;
        }
    }

    //
    //  Now setup the alt image with the origin/roi and other necessary
    //  information from our original device image.  Then, reset the members
    //  of the op to point at our alternate image.
    //
    alt_img->setOrigin(img->getOriginX(), img->getOriginY());
    alt_img->setRoi(img->getRoi());

    //
    //  Point the alt_img at the same storage as the original image.
    //
    if(alt_img->setStorageInfo(img) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    
    //
    //  Copy op queue information from img to alt_img and clear queue
    //  information from img.
    //
    if(alt_img->transferOpQueueInfo(img) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Generate the new name and reset the op number.
    //
    char buffer[8192];
    sprintf(buffer, "%s_%s", getOpName(),
            alt_img->getDeviceIO()->getDeviceManager()->getDeviceName());
    
    XilGlobalState*   xgs = XilGlobalState::getXilGlobalState();
    XilOpNumber       op_number;
    if((op_number = xgs->lookupOpNumber(buffer)) < 0) {
        return XIL_FAILURE;
    }
    setOpNumber(op_number);

    //
    //  Reset the source and destination images on the op...
    //
    setSrc(1, alt_img);
    setDst(1, alt_img);

    //
    //  Set the flag to indicate we've already mutated to the default display
    //  device.
    //
    mutatedToDefaultFlag = TRUE;

    return XIL_SUCCESS;
#endif /* else _WINDOWS */
}
