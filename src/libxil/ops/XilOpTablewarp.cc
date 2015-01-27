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
//  File:	XilOpTablewarp.cc
//  Project:	XIL
//  Revision:	1.14
//  Last Mod:	10:07:32, 03/10/00
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
#pragma ident	"@(#)XilOpTablewarp.cc	1.14\t00/03/10  "

#include "XilOpGeometricWarp.hh"

class XilOpTablewarp : public XilOpGeometricWarp {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
                  XilOpTablewarp(XilOpNumber            op_num,
                                 XilImage*              src_image,
                                 XilImage*              dst_image,
                                 XiliInterpolationType  type,
                                 XilInterpolationTable* h_table,
                                 XilInterpolationTable* v_table) :
        XilOpGeometricWarp(op_num, src_image, dst_image,
                           type, h_table, v_table)
    {
    }

    virtual              ~XilOpTablewarp()
    {
    }
};

static
XilOpGeometricWarp*
xili_tablewarp_op_create_func(XilOpNumber            op_num,
                              XilImage*              src_image,
                              XilImage*              dst_image,
                              XiliInterpolationType  type,
                              XilInterpolationTable* h_table,
                              XilInterpolationTable* v_table)
{
    return new XilOpTablewarp(op_num, src_image, dst_image,
                              type, h_table, v_table);
}

XilOp*
XilOpTablewarp::create(char  function_name[],
                       void* args[],
                       int   )
{
    static XilOpCache  tw_nn_op_cache(1);
    static XilOpCache  tw_bl_op_cache(1);
    static XilOpCache  tw_bc_op_cache(1);
    static XilOpCache  tw_gn_op_cache(1);
    static XilOpCache* tablewarp_op_caches[XILI_NUM_SUPPORTED_INTERPOLATIONS] =
        { &tw_nn_op_cache, &tw_bl_op_cache, &tw_bc_op_cache, &tw_gn_op_cache };

    return constructWarpOp(function_name, args,
                           tablewarp_op_caches, 2,
                           xili_tablewarp_op_create_func);
}
