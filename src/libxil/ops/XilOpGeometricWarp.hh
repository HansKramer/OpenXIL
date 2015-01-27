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
//  File:	XilOpGeometricWarp.hh
//  Project:	XIL
//  Revision:	1.13
//  Last Mod:	10:20:34, 03/10/00
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
#pragma ident	"@(#)XilOpGeometricWarp.hh	1.13\t00/03/10  "

#ifndef _XIL_OP_GEOMETRIC_WARP_HH
#define _XIL_OP_GEOMETRIC_WARP_HH

#include "XilOpGeometric.hh"

class XilOpGeometricWarp;

//
//  Function that calls "new" on the target op that's using
//  xili_construct_tablewarp_op() (i.e. horizontal, vertial, basic).
//
typedef XilOpGeometricWarp*
    (*XiliTablewarpOpCreateFuncPtr)(XilOpNumber            op_num,
                                    XilImage*              src_image,
                                    XilImage*              dst_image,
                                    XiliInterpolationType  type,
                                    XilInterpolationTable* h_table,
                                    XilInterpolationTable* v_table);

class XilOpGeometricWarp : public XilOpGeometric {
protected:
                         XilOpGeometricWarp(XilOpNumber            op_num,
                                            XilImage*              src_image,
                                            XilImage*              dst_image,
                                            XiliInterpolationType  type,
                                            XilInterpolationTable* h_table,
                                            XilInterpolationTable* v_table) :
        XilOpGeometric(op_num, src_image, dst_image, type, h_table, v_table)
    {
    }

    virtual              ~XilOpGeometricWarp()
    {
    }

    //
    //  Function to consolidate all of the warp op construction which is
    //  constant between the different tablewarp ops.
    //
    static XilOp*        constructWarpOp(const char*                  function_name,
                                         void*                        args[],
                                         XilOpCache*                  op_caches[],
                                         unsigned int                 num_warp_bands,
                                         XiliTablewarpOpCreateFuncPtr op_create_func);

    //
    //  Warp doesn't not support splitting on tile boundaries in the
    //  first source image.
    //
    //  TODO: 4/7/97 jlf  We can support splitting in the warp image which
    //                    could help at times.  
    //
    virtual XilStatus vSplitOnTileBoundaries(XilBoxList*)
    {
        return XIL_SUCCESS;
    }

    //
    //  A test to see if the pixels written by the current op covers the area
    //  written by the previous op.  See routine in XilOpPrivate.cc for more
    //  details.
    //
    //  For tablewarp, this is impossible since we don't analyze the contents
    //  of the warp image. 
    //
    virtual Xil_boolean thisOpCoversPreviousOp()
    {
        return FALSE;
    }

    //
    // Backward maps to the bounding box of the src
    // roi.
    //
    virtual XilStatus gsBackwardMap(XiliRect*    dst_rect,
				    XiliRect*    src_rect,
				    unsigned int src_number);

    //    
    //  Over-riding setBoxStorage map to set the source storage area to
    //  be bigger than the pixel area.
    //
    virtual XilStatus setBoxStorage(XiliRect*            rect,
                                    XilDeferrableObject* object,
                                    XilBox*              box);

    //
    // This is the same code for all the sub-classes and
    // sets up the op parameters used by all of the table
    // warp implementations. The returned value allows us
    // to know where any further parameters will be placed
    //
    virtual int setWarpParams();

    
    //
    // overloaded functions
    //
    virtual XilStatus    moveIntoObjectSpace(XiliRect*            rect,
                                             XilDeferrableObject* object);
   
    virtual XilStatus    moveIntoObjectSpace(XilRoi*              roi,
                                             XilDeferrableObject* object);
    

    virtual XilStatus    moveIntoGlobalSpace(XiliRect*            rect,
                                             XilDeferrableObject* object);
    
    virtual XilStatus    moveIntoGlobalSpace(XilRoi*              roi,
                                             XilDeferrableObject* object);
    
private:
    //
    //  Store the bounding box of the source roi
    //
    int          sroi_x;
    int          sroi_y;
    unsigned int sroi_w;
    unsigned int sroi_h;
    Xil_boolean  whole_image;

    //
    //  We also save the source width and height
    //
    unsigned int src_width;
    unsigned int src_height;
};

#endif // _XIL_OP_GEOMETRIC_WARP_HH
