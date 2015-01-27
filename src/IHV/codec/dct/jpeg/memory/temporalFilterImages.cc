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
//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:       temporalFilterImages.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:14:36, 03/10/00
//
//  Description:
//
//    Filters the image to produce a synthetic image that will
//    produce a much greater compression ratio since the "noise" has
//    been removed.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)temporalFilterImages.cc	1.6\t00/03/10  "


#include <math.h>
#include "xil/xilGPI.hh"
#include "XilDeviceCompressionJpeg.hh"
#include "CompressInfo.hh"

//
// TODO: Re-implement using the squares table //
#if 0
#include "SquaresTable.hh"
#endif

XilStatus 
XilDeviceCompressionJpeg::temporalFilterImages(
    Xil_unsigned8*  prvBuf,
    CompressInfo*   ci,
    Xil_unsigned8*  outBuf,
    int             lowFilterThresh,
    int             highFilterThresh)
{
    //
    // Precalculate the square of the filter thresholds and the delta
    //
    int   lowFilterThreshSquare = lowFilterThresh * lowFilterThresh; 
    int   highFilterThreshSquare = highFilterThresh * highFilterThresh;
    float deltaThresh = highFilterThresh - lowFilterThresh;

    int dist;
    unsigned int prv_ss = ci->image_width*3;
    unsigned int out_ss = ci->image_width*3;
    unsigned int cur_ss = ci->image_ss;
    unsigned int prv_ps = 3;
    unsigned int out_ps = 3;
    unsigned int cur_ps = ci->image_ps;

    Xil_unsigned8* prv_scan = prvBuf;
    Xil_unsigned8* out_scan = outBuf;
    Xil_unsigned8* cur_scan = (Xil_unsigned8*)ci->image_dataptr;
    for(int r=0; r<(int)ci->image_height; r++) {
        Xil_unsigned8* prv_pixel = prv_scan;
        Xil_unsigned8* out_pixel = out_scan;
        Xil_unsigned8* cur_pixel = cur_scan;
        for(int c=0; c<(int)ci->image_width; c++) {
            int b0_dif = cur_pixel[0] - prv_pixel[0];
            int b1_dif = cur_pixel[1] - prv_pixel[1];
            int b2_dif = cur_pixel[2] - prv_pixel[2];

            dist = b0_dif*b0_dif + b1_dif*b1_dif + b2_dif*b2_dif;

            //
            //  If I'm below the low threshold, then I select the
            //  previous image data.
            //
            //  If I'm below the high threshold, then I
            //  interpolate between the previous and the current
            //  image and then store a blend into the previous
            //  image.
            //
            //  Otherwise, I select the given image data
            //
            if(dist < lowFilterThreshSquare) {
                out_pixel[0] = prv_pixel[0];
                out_pixel[1] = prv_pixel[1];
                out_pixel[2] = prv_pixel[2];
            } else if(dist < highFilterThreshSquare) {
                float t = sqrt((double) dist);
                t = (t - (float) lowFilterThresh) / (float) deltaThresh;
                float t1 = 1.0 - t;

                out_pixel[0] = t*cur_pixel[0] + t1*prv_pixel[0] + 0.5;
                out_pixel[1] = t*cur_pixel[1] + t1*prv_pixel[1] + 0.5;
                out_pixel[2] = t*cur_pixel[2] + t1*prv_pixel[2] + 0.5;

                prv_pixel[0] = (3*prv_pixel[0] + cur_pixel[0] + 2) / 4;
                prv_pixel[1] = (3*prv_pixel[1] + cur_pixel[1] + 2) / 4;
                prv_pixel[2] = (3*prv_pixel[2] + cur_pixel[2] + 2) / 4;
            } else {
                out_pixel[0] = cur_pixel[0];
                out_pixel[1] = cur_pixel[1];
                out_pixel[2] = cur_pixel[2];

                prv_pixel[0] = cur_pixel[0];
                prv_pixel[1] = cur_pixel[1];
                prv_pixel[2] = cur_pixel[2];
            }

            cur_pixel += cur_ps;
            prv_pixel += prv_ps;
            out_pixel += out_ps;
        }
        cur_scan += cur_ss;
        prv_scan += prv_ss;
        out_scan += out_ss;
    }

    return XIL_SUCCESS;
}
