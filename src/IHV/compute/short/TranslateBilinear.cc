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
//  File:	TranslateBilinear.cc
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:12:19, 03/10/00
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
#pragma ident	"@(#)TranslateBilinear.cc	1.5\t00/03/10  "


#include "XilDeviceManagerComputeSHORT.hh"
#include "ComputeInfo.hh"
#include "xili_geom_utils.hh"

XilStatus
XilDeviceManagerComputeSHORT::TranslateBilinear(XilOp*       op,
					      unsigned        ,
					      XilRoi*      roi,
					      XilBoxList*  bl)
{
    XilStatus status;
    AffineData ad;

    ad.op  = op;
    ad.roi = roi;
   
    float	xoffset;
    float       yoffset;
    
    op->getParam(1, &xoffset);
    op->getParam(2, &yoffset);
    //
    // construct affine matrix
    //
    float matrix[6];		// Equivalent affine matrix for translate

    matrix[0] = 1.0;
    matrix[1] = 0.0;
    matrix[2] = 0.0;
    matrix[3] = 1.0;
    matrix[4] = xoffset;	
    matrix[5] = yoffset;

    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    ad.matrix = matrix;

    status = affineBilinear(bl, ad);

    return status;
}

