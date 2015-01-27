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
//  File:	XilOpCapture.cc
//  Project:	XIL
//  Revision:	1.25
//  Last Mod:	10:07:39, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilOpCapture.cc	1.25\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XiliOpUtils.hh"
#include "XilOpIO.hh"

class XilOpCapture : public XilOpIO {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);

    //
    //  Overload of setBoxStorage which takes into account the
    //  offset between the realImage and the controllingImage.
    //  The box and the intersectedRoi must be translated because
    //  the compute routine only works on the controlling image
    //
    XilStatus     setBoxStorage(XiliRect*            rect,
                                XilDeferrableObject* object,
                                XilBox*              box);

    //
    //  Overload generation of intersectedRoi because a capture
    //  should only update the region that is actually needed
    //  in the next operation.
    //
    XilStatus     generateIntersectedRoi();

    //
    //  Clips the given rect (which is expected to be valid in the given
    //  object's image space) to tile number in the specified object.
    //  This is overloaded to take into account possible translation
    //  done on the ROI from which "rect" is taken.
    //
    XilStatus     clipToTile(XilDeferrableObject* defobj,
                             XilTileNumber        tile_number,
                             XiliRect*            rect);
    

    Xil_boolean   isIOOperation(XiliOpIOType type)
    {
        if(type == XILI_OP_IO_TYPE_CAPTURE || type == XILI_OP_IO_TYPE_ANY) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    //
    //  TODO:  2/14/97 jlf  Remove this flushOnInsert() case for PUSH ops when
    //                      push operations work with tiling. 
    //
    Xil_boolean   flushOnInsert() const
    {
        return ioType == XIL_DEVICE_IO_PUSH;
    }
    
    const char*   getOpName()
    {
        return "capture";
    }
    
protected:
                  XilOpCapture(XilOpNumber     op_num,
                               XilOp*          real_op,
                               unsigned int    branch_num,
                               XilDeviceIOType io_type) :
                      XilOpIO(op_num, real_op)
    {
        ioType = io_type;

        //
        //  Is the capture being done into the source of the subsequent
        //  operation or the destination of the subsequent operation (as in
        //  the case of preparing the destination for a sub-band display).
        //
        //  The branch number may be for a destination image if it's greater
        //  than the number of sources (as in the case of preparing the
        //  destination for a sub-band display).
        //
        if(branch_num > realOp->getNumSrcs()) {
            realImage = realOp->getDstImage(branch_num - realOp->getNumSrcs());
        } else {
            realImage = realOp->getSrcImage(branch_num);
        }

        //
        //  Initialize the isNewFrameFlag to TRUE.
        //
        setIsNewFrameFlag(TRUE);
    }

                  ~XilOpCapture()
    {
        getDstImage(1)->setStorageValidFlag(TRUE);
    }

    XilDeviceIOType ioType;
};

//
//  Overload generateIntersectedRoi() so we use the intersected ROI of the
//  operation following the capture as our dest ROI.
//
//  Capture operations are only supposed to update the areas touched by the
//  following operation when updating the controlling image -- not the entire area
//  represented by the device.
//
XilStatus
XilOpCapture::generateIntersectedRoi()
{

    XilRoi* int_roi;

    if((int_roi = getIntersectedRoi()) == NULL) {
        return XIL_FAILURE;
    }

    //
    //  Get the global space roi of the operation's source image -- this IS
    //  our intersected ROI.
    //
    //  You don't want to capture any more than you'll ultimately use.
    //
    XilRoi* dstop_src_im_roi = realImage->getGlobalSpaceRoi();
    if(dstop_src_im_roi == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-401", FALSE);
        return XIL_FAILURE;
    }

    *int_roi = *dstop_src_im_roi;

    //
    //  Indicate if a ROI had been translated, then it has no longer been
    //  translated.
    //
    translatedROI = FALSE;

    return XIL_SUCCESS;
}

XilStatus
XilOpCapture::setBoxStorage(XiliRect*            rect,
                            XilDeferrableObject* ,
                            XilBox*              box)
{
    *box = *rect;

    //
    //  The incoming rect will be valid for the source of our real
    //  operation.  We will convert the rect from the child image (a
    //  potential) used in the real operation into the controlling image.
    //
    //  TODO: 11/11/96 jlf  Bug -- getSrcImage may not be #1 but something else.
    //
    if(realImage != getDstImage(1)) {
        if(! translatedROI) {
            //
            //  They're not the same so the BOX needs to be translated to be in
            //  the controlling image...
            //
            unsigned int src_x_offset;
            unsigned int src_y_offset;
            unsigned int src_b_offset;
            controllingImage->getChildOffsets(&src_x_offset,
                                              &src_y_offset,
                                              &src_b_offset);

            unsigned int dstop_src_x_offset;
            unsigned int dstop_src_y_offset;
            unsigned int dstop_src_b_offset;
            realImage->getChildOffsets(&dstop_src_x_offset,
                                                    &dstop_src_y_offset,
                                                    &dstop_src_b_offset);

            if(src_x_offset != dstop_src_x_offset ||
               src_y_offset != dstop_src_y_offset) {
                translateX = dstop_src_x_offset - src_x_offset;
                translateY = dstop_src_y_offset - src_y_offset;

                box->offset(translateX, translateY);
            
                if(! translatedROI) {
                    XilRoi* int_roi = getIntersectedRoi();

                    if(int_roi == NULL) {
                        return XIL_FAILURE;
                    }

                    if(int_roi->translate_inplace(translateX,
                                                  translateY) ==
                       XIL_FAILURE) {
                        return XIL_FAILURE;
                    }

                    translatedROI = TRUE;
                }
            }
        } else if(translateX != 0 || translateY != 0) {
            box->offset(translateX, translateY);
        }
    }

    if(controllingImage->setBoxStorage(box) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	clipToTile()
//
//  Description:
//
//  Clips the given rect (which is expected to be valid in the given
//  object's image space) to tile number in the specified object.
//  This is overloaded to take into account possible translation
//  done on the ROI from which "rect" is taken.
//	
//------------------------------------------------------------------------
XilStatus
XilOpCapture::clipToTile(XilDeferrableObject* defobj,
                         XilTileNumber        tile_number,
                         XiliRect*            rect)
{
    if(translatedROI) {
        //
        //  We know that the rect was taken from the bbox of
        //  the intersected ROI. Since the intersectedRoi
        //  was moved into controlling image space (as flagged
        //  by "translatedROI") we need to undo that translation so
        //  that the clip can happen properly through the defobj.
        //
        rect->translate(-translateX,-translateY);
    }
    return (defobj->clipToTile(tile_number, rect));
}

XilOp*
XilOpCapture::create(char* ,
		     void* args[],
		     int   )
{
    XilImage*    img        = (XilImage*)args[0];
    XilOp*       real_op    = (XilOp*)args[1];
    unsigned int branch_num = (unsigned int)args[2];

    //
    //  Is the image valid
    //
    if(img == NULL) {
	XIL_ERROR(NULL, XIL_ERROR_USER, "di-207", TRUE);
	return NULL;
    }

    //
    //  Verify it a device image...
    //
    XilDeviceIO* io_dev = img->getXilDeviceIO();
    if(io_dev == NULL) {
	XIL_ERROR(img->getSystemState(), XIL_ERROR_USER, "di-TODO", TRUE);
	return NULL;
    }

    XilOpNumber     op_number = -1;
    XilDeviceIOType io_type   = io_dev->getType();

    //
    //  Only PULL devices have a capture() call and are executed like other
    //  compute operations.  PUSH devices don't have an op number because
    //  they're handled specially in the XIL core.
    //
    if(io_type == XIL_DEVICE_IO_PULL) {
        //
        // TODO: 1/11/96 jlf  This should not be a fixed array.
        //
        //   But it shouldn't be 'newed' by default.  Probably should test size
        //   and only new if > max.
        //
        char buffer[8192];
        sprintf(buffer, "capture_%s", io_dev->getDeviceManager()->getDeviceName());

        //
        //  Lookup the op number and store the result in the cache...
        //
        static XilOpCache op_cache;
        
        XilGlobalState*   xgs = XilGlobalState::getXilGlobalState();
        if((op_number = op_cache.set(XIL_BIT, xgs->lookupOpNumber(buffer))) < 0) {
            XIL_ERROR(img->getSystemState(),
                      XIL_ERROR_CONFIGURATION,"di-5",TRUE);
            return NULL;
        }
    }

    //
    //  Create the op and set its arguments.
    //
    XilOpCapture* op = new XilOpCapture(op_number, real_op, branch_num, io_type);
    if(op == NULL) {
        XIL_ERROR(img->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    if(io_type == XIL_DEVICE_IO_PUSH) {
        //
        //  If the device is a "push" device, we need to mark this op as
        //  "forward mapping".
        //
        op->setForwardMapping();
    }

    //
    //  In-place operation with the controlling image...
    //
    //  For XIL 1.3, we don't pass children onto the display and capture
    //  routines, just the controlling image that we were given.
    //
    XilImage* controlling_image = io_dev->getControllingImage();
    op->setSrc(1, controlling_image);
    op->setDst(1, controlling_image);

    //
    //  Set up device offset parameters.
    //
    //  We tell the device the band subset that it's actually copying.  The
    //  device can then decide if it wants to perform the whole copy or just
    //  the subset defined by the child.
    //
    unsigned int ci_offset_x;
    unsigned int ci_offset_y;
    unsigned int ci_offset_band;
    controlling_image->getChildOffsets(&ci_offset_x,
                                       &ci_offset_y,
                                       &ci_offset_band);
    
    op->controllingImage = controlling_image;
    op->realImage        = img;

    unsigned int img_offset_x;
    unsigned int img_offset_y;
    unsigned int img_offset_band;
    img->getChildOffsets(&img_offset_x, &img_offset_y, &img_offset_band);

    op->setParam(1, img->getNumBands());
    op->setParam(2, img_offset_band - ci_offset_band);

    return op;
}
