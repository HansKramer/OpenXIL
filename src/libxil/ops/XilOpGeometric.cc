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
//  File:	XilOpGeometric.cc
//  Project:	XIL
//  Revision:	1.18
//  Last Mod:	10:07:31, 03/10/00
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
#pragma ident	"@(#)XilOpGeometric.cc	1.18\t00/03/10  "

#include <stdlib.h>
#include <string.h>
#include <xil/xilGPI.hh>
#include "XilOpGeometric.hh"
#include "XiliOpUtils.hh"

XilOpGeometric::XilOpGeometric(XilOpNumber            op_number,
                               XilImage*              src_image,
                               XilImage*              dst_image,
                               XiliInterpolationType  interp_type,
                               XilInterpolationTable* h_table,
                               XilInterpolationTable* v_table) :
    XilOp(op_number)
{
    //
    //  Store the interpolation type and images.
    //
    interpolationType = interp_type;
    srcImage          = src_image;
    dstImage          = dst_image;

    //
    //  Store their origins.
    //
    srcImage->getOrigin(&src_ox, &src_oy);
    dstImage->getOrigin(&dst_ox, &dst_oy);

    //
    //  Source and destnation image edges in global and image space for simple
    //  reference later. 
    //
    unsigned int src_width  = srcImage->getWidth();
    unsigned int src_height = srcImage->getHeight();
    
    srcgs_X1 = -src_ox;
    srcgs_Y1 = -src_oy;
    srcgs_X2 = src_width  - 1 - src_ox;
    srcgs_Y2 = src_height - 1 - src_oy;

    srcos_X1 = 0;
    srcos_Y1 = 0;
    srcos_X2 = src_width  - 1;
    srcos_Y2 = src_height - 1;

    unsigned int dst_width  = dstImage->getWidth();
    unsigned int dst_height = dstImage->getHeight();

    dstgs_X1 = -dst_ox;
    dstgs_Y1 = -dst_oy;
    dstgs_X2 = dst_width  - 1 - dst_ox;
    dstgs_Y2 = dst_height - 1 - dst_oy;

    //
    //  Set up edges for different interpolation types.  We add these to take
    //  the edges into account (i.e. shrink the source) and subtract these to
    //  extend the source (i.e. grow the source) box.
    //
    switch(interp_type) {
      case XiliNearest:
        //
        //  No extra storage needed for nearest.
        //
        leftEdge   =  0.0;
        topEdge    =  0.0;
        rightEdge  =  0.0;
        bottomEdge =  0.0;
        break;

      case XiliBilinear:
        //
        //  Need extra storage of one pixel along right & bottom.
        //
        leftEdge   =  0.0;
        topEdge    =  0.0;
        rightEdge  = -1.0;
        bottomEdge = -1.0;
        break;

      case XiliBicubic:
        //
        //  Need extra storage of two pixels along right & bottom and one
        //  pixel along top & left.
        //
        leftEdge   =  1.0;
        topEdge    =  1.0;
        rightEdge  = -2.0;
        bottomEdge = -2.0;
        break;

      case XiliGeneral:
      {
          //
          //  These are set based on the kernel dimensions. 
          //
          //  The key values are computed from the width and height of the
          //  kernel dimensions.
          //
          unsigned int keyX = (h_table->getKernelSize() - 1)/2;
          unsigned int keyY = (v_table->getKernelSize() - 1)/2;

          leftEdge   = keyX;
          topEdge    = keyY;
          rightEdge  = -(int)(h_table->getKernelSize() - keyX - 1);
          bottomEdge = -(int)(v_table->getKernelSize() - keyY - 1);
      }
      break;
    }
}

