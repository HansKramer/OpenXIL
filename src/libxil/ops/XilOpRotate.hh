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
//  File:	XilOpRotate.hh
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:20:41, 03/10/00
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
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident  "@(#)XilOpRotate.hh	1.4\t00/03/10  "

#include "XilOpGeometricAffine.hh"

class XilOpRotate : public XilOpGeometricAffine {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);

    //
    //  A test to see if the pixels written by the current op covers the area
    //  written by the previous op.  See routine in XilOpPrivate.cc for more
    //  details.
    //
    //  TODO: 10/8/96 jlf  Implement this routine so it will catch the cases
    //                     when is op covers previous op instead of always
    //                     assuming this op will not cover the previous op.
    //
    Xil_boolean   thisOpCoversPreviousOp()
    {
        return FALSE;
    }

protected:
                  XilOpRotate(XilOpNumber            op_num,
                              XilImage*              src_image,
                              XilImage*              dst_image,
                              XiliInterpolationType  type,
                              AffineTr               affine_tr,
                              XilInterpolationTable* h_table,
                              XilInterpolationTable* v_table) :
        XilOpGeometricAffine(op_num, src_image, dst_image,
                             type, affine_tr, h_table, v_table)
    {
    }


    virtual       ~XilOpRotate()
    {
    }
};
