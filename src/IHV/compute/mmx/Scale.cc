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
//  File:       Scale.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:13:45, 03/10/00
//
//  Description:
//
//    Call MMX scale function. IPL supplies separate zoom and
//    decimate functions, but not a general 2-axis scale.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Scale.cc	1.6\t00/03/10  "

#include "XilDeviceManagerComputeMMXBYTE.hh"
#include "ComputeInfoMMX.hh"

XilStatus
XilDeviceManagerComputeMMXBYTE::ScaleNearest(XilOp*       op,
                                             unsigned     op_count,
                                             XilRoi*      roi,
                                             XilBoxList*  bl)
{
    // 
    // Declare a pointer to function so we can set it to either
    // iplZoom or iplDecimate, depending on the scale factors.
    //
    void (*scaleFunc)(IplImage*, IplImage*, 
                      int, int, int, int, 
                      int, IplCoord*);

    //
    // Get the X and Y scale factors from the op
    //
    float       xscale, yscale;

    op->getParam(1, (void **)&xscale);
    op->getParam(2, (void **)&yscale);

    //
    // Determine whether its a zoom or decimate and set the
    // function pointer accordingly. Reject mixed zoom/decimate.
    // The op has already taken care of detecting a 1.0/1.0 scale
    // and converting it to a copy.
    //
    if(xscale >= 1.0F && yscale >= 1.0F) {
        scaleFunc = iplZoom;
    } else if(xscale < 1.0F && yscale < 1.0F) {
        scaleFunc = iplDecimate;
    } else {
        //
        // IPL cannot handle zoom in one axis and decimate in the other
        // Let the memory pipeline do this.
        //
        return XIL_FAILURE;
    }

    ComputeInfoMMX  ci(op, op_count, roi, bl);

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            //
            // Do some initial image setup.
            // We'll modify this later to take account of
            // the scale factors.
            //
            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            //
            // Backward map the dst rect's upper-left corner
            // into the source, so we can get the src rect origin
            //
            double src_x, src_y;
            int    dst_w = ci.xsize;
            int    dst_h = ci.ysize;

            //
            // Src dimensions are derived by inverse scaling the dst
            //
            int    src_w = (int) (dst_w / xscale);
            int    src_h = (int) (dst_h / yscale);

            //
            // Find src rect origin by backward mapping the dst rect origin 
            //
            op->backwardMap(ci.getDestBox(), (double)ci.x, (double)ci.y,
                            ci.getSrc1Box(), &src_x, &src_y);

            //
            // Modify the src image description, since the
            // existing one is based on the dst box size.
            //
            ci.mmxSrc1.height = src_h;
            ci.mmxSrc1.width  = ci.mmxSrc1.widthStep / ci.mmxSrc1.nChannels;
            ci.mmxSrc1.imageSize = ci.mmxSrc1.widthStep * src_h;
            iplSetROI(ci.mmxSrc1.roi, 0, 
                      (int)src_x, (int)src_y, src_h, src_w);

            //
            // Call the appropriate Ipl scaling function 
            // via the function pointer.
            //
            scaleFunc(&ci.mmxSrc1, &ci.mmxDst, 
                      dst_w, src_w, dst_h, src_h, 
                      IPL_INTER_NN, NULL);

            if(IPL_ERRCHK("ScaleNN", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}

XilStatus
XilDeviceManagerComputeMMXBYTE::ScaleBilinear(XilOp*       op,
                                              unsigned     op_count,
                                              XilRoi*      roi,
                                              XilBoxList*  bl)
{
    // 
    // Declare a pointer to function so we can set it to either
    // iplZoom or iplDecimate, depending on the scale factors.
    //
    void (*scaleFunc)(IplImage*, IplImage*, 
                      int, int, int, int, 
                      int, IplCoord*);

    //
    // Get the X and Y scale factors from the op
    //
    float       xscale, yscale;

    op->getParam(1, (void **)&xscale);
    op->getParam(2, (void **)&yscale);

    //
    // Determine whether its a zoom or decimate and set the
    // function pointer accordingly. Reject mixed zoom/decimate.
    //
    if(xscale >= 1.0F && yscale >= 1.0F) {
        scaleFunc = iplZoom;
    } else if(xscale < 1.0F && yscale < 1.0F) {
        scaleFunc = iplDecimate;
    } else {
        //
        // IPL cannot handle zoom in one axis and decimate in the other
        //
        return XIL_FAILURE;
    }

    ComputeInfoMMX  ci(op, op_count, roi, bl);

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            //
            // Do some initial image setup.
            // We'll modify this later to take account of
            // the scale factors.
            //
            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            //
            // Backward map the dst rect's upper-left corner
            // into the source, so we can get the src rect origin
            //
            double src_x, src_y;
            int    dst_w = ci.xsize;
            int    dst_h = ci.ysize;

            //
            // Src dimensions are derived by inverse scaling the dst
            //
            int    src_w = (int) (dst_w / xscale);
            int    src_h = (int) (dst_h / yscale);

            //
            // Find src rect origin by backward mapping the dst rect origin 
            //
            op->backwardMap(ci.getDestBox(), (double)ci.x, (double)ci.y,
                            ci.getSrc1Box(), &src_x, &src_y);

            //
            // Modify the src image description, since the
            // existing one is based on the dst box size.
            //
            ci.mmxSrc1.height = src_h;
            ci.mmxSrc1.width  = ci.mmxSrc1.widthStep / ci.mmxSrc1.nChannels;
            ci.mmxSrc1.imageSize = ci.mmxSrc1.widthStep * src_h;
            iplSetROI(ci.mmxSrc1.roi, 0, 
                      (int)src_x, (int)src_y, src_h, src_w);

            //
            // Call the appropriate Ipl scaling function 
            // via the function pointer.
            //
            scaleFunc(&ci.mmxSrc1, &ci.mmxDst, 
                      dst_w, src_w, dst_h, src_h, 
                      IPL_INTER_LINEAR, NULL);

            if(IPL_ERRCHK("ScaleBL", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}

XilStatus
XilDeviceManagerComputeMMXBYTE::ScaleBicubic(XilOp*       op,
                                             unsigned     op_count,
                                             XilRoi*      roi,
                                             XilBoxList*  bl)
{
    // 
    // Declare a pointer to function so we can set it to either
    // iplZoom or iplDecimate, depending on the scale factors.
    //
    void (*scaleFunc)(IplImage*, IplImage*, 
                      int, int, int, int, 
                      int, IplCoord*);

    //
    // Get the X and Y scale factors from the op
    //
    float       xscale, yscale;

    op->getParam(1, (void **)&xscale);
    op->getParam(2, (void **)&yscale);

    //
    // Determine whether its a zoom or decimate and set the
    // function pointer accordingly. Reject mixed zoom/decimate.
    //
    if(xscale >= 1.0F && yscale >= 1.0F) {
        scaleFunc = iplZoom;
    } else if(xscale < 1.0F && yscale < 1.0F) {
        scaleFunc = iplDecimate;
    } else {
        //
        // IPL cannot handle zoom in one axis and decimate in the other
        //
        return XIL_FAILURE;
    }

    ComputeInfoMMX  ci(op, op_count, roi, bl);

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            //
            // Do some initial image setup.
            // We'll modify this later to take account of
            // the scale factors.
            //
            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            //
            // Backward map the dst rect's upper-left corner
            // into the source, so we can get the src rect origin
            //
            double src_x, src_y;
            int    dst_w = ci.xsize;
            int    dst_h = ci.ysize;

            //
            // Src dimensions are derived by inverse scaling the dst
            //
            int    src_w = (int) (dst_w / xscale);
            int    src_h = (int) (dst_h / yscale);

            //
            // Find src rect origin by backward mapping the dst rect origin 
            //
            op->backwardMap(ci.getDestBox(), (double)ci.x, (double)ci.y,
                            ci.getSrc1Box(), &src_x, &src_y);

            //
            // Modify the src image description, since the
            // existing one is based on the dst box size.
            //
            ci.mmxSrc1.height = src_h;
            ci.mmxSrc1.width  = ci.mmxSrc1.widthStep / ci.mmxSrc1.nChannels;
            ci.mmxSrc1.imageSize = ci.mmxSrc1.widthStep * src_h;
            iplSetROI(ci.mmxSrc1.roi, 0, 
                      (int)src_x, (int)src_y, src_h, src_w);

            //
            // Call the appropriate Ipl scaling function 
            // via the function pointer.
            //
            scaleFunc(&ci.mmxSrc1, &ci.mmxDst, 
                      dst_w, src_w, dst_h, src_h, 
                      IPL_INTER_CUBIC, NULL);

            if(IPL_ERRCHK("ScaleBC", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}


