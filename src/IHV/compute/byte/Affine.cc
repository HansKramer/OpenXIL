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
//  File:	Affine.cc
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	10:10:29, 03/10/00
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
#pragma ident	"@(#)Affine.cc	1.3\t00/03/10  "

#include "XilDeviceManagerComputeBYTE.hh"
#include "ComputeInfo.hh"
#include "ComputeBYTEUtil.h"

XilStatus
XilDeviceManagerComputeBYTE::Affine(XilOp*       op,
				    unsigned     op_count,
				    XilRoi*      roi,
				    XilBoxList*  bl)
{
    ComputeInfoBYTE  ci(op, op_count, roi, bl);

    if(ci.isOK() == FALSE) {
        return XIL_CONTINUE;
    }

    float*	matrix;
    op->getParam(1, (void **)&matrix);

for(int i=0; i<6; i++)
printf("matrix[%d] = %f\n", i, matrix[i]);

    return ci.returnValue;
}
