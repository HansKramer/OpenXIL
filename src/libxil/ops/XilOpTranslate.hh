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
//  File:	XilOpTranslate.hh
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:20:45, 03/10/00
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
#pragma ident  "@(#)XilOpTranslate.hh	1.5\t00/03/10  "

#include "XilOpGeometricAffine.hh"

class XilOpTranslate : public XilOpGeometricAffine {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);

    //
    //  Eventhough translate can be optimized to use rects versus full-blown
    //  convex regions due to the fact it never warps the image, it still must
    //  be done in floating point.  We have optimized all the affine code and
    //  convex region handling code so rectangles incur no convex region
    //  overhead...they're handled as rects.  Thus, there is no significant
    //  benefit to reimplementing the methods provided by
    //  XilOpGeometricAffine.
    //

protected:
                  XilOpTranslate(XilOpNumber            op_num,
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


    virtual       ~XilOpTranslate()
    {
    }
};
