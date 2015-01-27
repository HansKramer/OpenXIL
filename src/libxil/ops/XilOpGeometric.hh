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
//  File:	XilOpGeometric.hh
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:20:33, 03/10/00
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
#pragma ident	"@(#)XilOpGeometric.hh	1.12\t00/03/10  "

#ifndef _XIL_OP_GEOMETRIC_HH
#define _XIL_OP_GEOMETRIC_HH

#include <xil/xilGPI.hh>
#include "XiliOpUtils.hh"

class XilOpGeometric : public XilOp {
protected:
                          XilOpGeometric(XilOpNumber            op_number,
                                         XilImage*              src_image,
                                         XilImage*              dst_image,
                                         XiliInterpolationType  interp_type,
                                         XilInterpolationTable* h_table,
                                         XilInterpolationTable* v_table);

    virtual               ~XilOpGeometric()
    {
    }

    //
    //  Extra source padding required for the interpolation types.
    //
    //  See comment in constructor for their meaning/use.
    //
    double                topEdge;
    double                bottomEdge;
    double                leftEdge;
    double                rightEdge;
    
    //
    //  Enum to store the interpolation type.
    //
    XiliInterpolationType interpolationType;

    //
    //  Often-utilized image information.
    //
    //  Source and destination image of the operation.
    //
    XilImage*             srcImage;
    XilImage*             dstImage;

    //
    //  Image origins.
    //
    float                 src_ox;
    float                 src_oy;
    float                 dst_ox;
    float                 dst_oy;

    //
    //  Source image edges in global space and in object space.
    //
    double                srcgs_X1;
    double                srcgs_Y1;
    double                srcgs_X2;
    double                srcgs_Y2;

    double                srcos_X1;
    double                srcos_Y1;
    double                srcos_X2;
    double                srcos_Y2;

    double                dstgs_X1;
    double                dstgs_Y1;
    double                dstgs_X2;
    double                dstgs_Y2;
};

inline
XiliInterpolationType
xili_get_interpolation_type(const char* interpolation)
{
    XiliInterpolationType interp_type;

    if(interpolation == NULL) {
        interp_type = XiliUnsupportedInterpolation;
    } else if(strncmp(interpolation, "nearest", 7) == 0) {
        interp_type = XiliNearest;
    } else if(strncmp(interpolation, "bilinear", 8) == 0) {
        interp_type = XiliBilinear;
    } else if(strncmp(interpolation, "bicubic", 7) == 0) {
        interp_type = XiliBicubic;
    } else if(strncmp(interpolation, "general", 7) == 0) {
        interp_type = XiliGeneral;
    } else {
        interp_type = XiliUnsupportedInterpolation;
    }

    return interp_type;
}

#endif // _XIL_OP_GEOMETRIC_HH
