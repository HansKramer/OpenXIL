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
//  File:	ScaleBilinear.cc
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:09:40, 03/10/00
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
#pragma ident	"@(#)ScaleBilinear.cc	1.5\t00/03/10  "

#include "XilDeviceManagerComputeBIT.hh"
#include "ComputeInfo.hh"
#include "XiliUtils.hh"
#include "xili_geom_utils.hh"


XilStatus
XilDeviceManagerComputeBIT::ScaleBilinear(XilOp*       op,
					   unsigned        ,
					   XilRoi*      roi,
					   XilBoxList*  bl)
{
    XilStatus status;
    AffineData ad;

    ad.op = op;
    ad.roi = roi;
    //
    //  Get the transformation matrix. Translational part of
    //  the matrix contains destination origin, currently
    //  this is not supplied by the core.
    //
    float	xscale, yscale;
    
    op->getParam(1, &xscale);
    op->getParam(2, &yscale);
    //
    // construct affine matrix
    //
    float matrix[6];		// Equivalent affine matrix for scale

    matrix[0] = xscale;
    matrix[1] = 0.0;
    matrix[2] = 0.0;
    matrix[3] = yscale;
    matrix[4] = 0.0;	
    matrix[5] = 0.0;     

    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    ad.matrix = matrix;

    if (XILI_FLT_EQ(xscale, 2.0) && XILI_FLT_EQ(yscale, 2.0))
	status = scale_2x_BL(bl, ad);
    else if (XILI_FLT_EQ(xscale, 0.5) && XILI_FLT_EQ(yscale, 0.5)) 
	status = scale_05x_BL(bl, ad);
    else
	status = affineBilinear(bl, ad);

    return status;  

}


XilStatus
XilDeviceManagerComputeBIT::scale_2x_BL(XilBoxList* bl,
					AffineData  ad)
{
    //
    // TODO, implement optimization for scaling by two
    //
    return affineBilinear(bl, ad);
  
}


XilStatus
XilDeviceManagerComputeBIT::scale_05x_BL(XilBoxList* bl,
					 AffineData  ad)
{
    //
    // TODO, implement optimization for scaling by a half
    //
    return affineBilinear(bl, ad);
  
}
