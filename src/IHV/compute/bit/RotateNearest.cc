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
//  File:	RotateNearest.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:09:43, 03/10/00
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
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)RotateNearest.cc	1.6\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"
#include "xili_geom_utils.hh"

XilStatus
XilDeviceManagerComputeBIT::RotateNearest(XilOp*       op,
					   unsigned       ,
					   XilRoi*      roi,
					   XilBoxList*  bl)
{
    XilStatus status;
    AffineData ad;
    ad.op  = op;
    ad.roi = roi;
    //
    //  Get the transformation matrix. Translational part of
    //  the matrix contains destination origin, currently
    //  this is not supplied by the core. src_origin
    //  coordinates are passed as the 2nd and 3rd parameter.
    //
    float	angle;		// angle of rotation (clockwise)

    op->getParam(1, &angle);

    //
    // construct affine matrix
    //

    float matrix[6];		// Equivalent affine matrix for rotate
    float det;			// rotation determinant 
    float costheta;
    float sintheta;    

    costheta = cos(angle);
    sintheta = sin(angle);
    matrix[0] = matrix[3] = costheta;
    matrix[1] = -sintheta;
    matrix[2] = sintheta;
    //
    // this might have to change because the center is not always (0,0)
    //
    det =  matrix[0] * matrix[3] - matrix[2] * matrix[1];
    if (XILI_FLT_EQ(det, 0.0))
	return XIL_SUCCESS;

    
    float xo, yo;

    op->getParam(2, &xo);
    op->getParam(3, &yo);
    
    matrix[4] = xo;		
    matrix[5] = yo;

    //
    // Get the basic data, assuming that the src image is a BYTE image.
    // roi is the complete image ROI. This means that src and dst ROI's are
    // already taken into account, and the roi passed is the intersection
    // of these. 
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
  
    ad.matrix = matrix;
    //
    // possible accelerations here (depending on angle)
    //
    //
    // If nothing else works, call affine transform
    //
    status = affineNearest(bl, ad);

    return status;

}
