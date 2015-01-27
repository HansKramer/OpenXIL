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
//  File:       XilOpCopy.hh
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:20:35, 03/10/00
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
//      COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilOpCopy.hh	1.7\t00/03/10  "

#ifndef _XIL_OP_COPY_HH
#define _XIL_OP_COPY_HH

#include "XilOpPoint.hh"

class XilOpCopy : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int   count);

    //
    //  For now, if the source and destination are children of the same
    //  parent, we mark ourselves as "cannot be split" so the library doesn't
    //  split the operation.
    //
    Xil_boolean   canBeSplit();
    XilStatus     vSplitOnTileBoundaries(XilBoxList* box_list);

#ifdef XILI_COPY_REORDERING_IMPLEMENTED
    //
    //  Permits the op to reorder the processing of tiles by the core.  This
    //  is only called if reorderTiles is set.
    //
    XilStatus     reorderTileProcessing(XilTileList* old_list,
                                        XilTileList* new_list);
#endif

    _XIL_NEW_DELETE_OVERLOAD_PUBLIC(XilOpCopy)

protected:
                  XilOpCopy(XilOpNumber opnum) :
                      XilOpPoint(opnum)
    {
    }

                  ~XilOpCopy()
    {
    }

private:
#ifdef XILI_COPY_REORDERING_IMPLEMENTED
    //
    //  Reordering variables...
    //
    enum XiliCopyOrder {
        XILI_COPY_ORDER_L2R_T2B,
        XILI_COPY_ORDER_R2L_T2B,
        XILI_COPY_ORDER_L2R_B2T,
        XILI_COPY_ORDER_R2L_B2T
    };

    Xil_boolean   reorderingSet;
    XiliCopyOrder reorderDir;
#endif

    Xil_boolean   copyCanBeSplit;
    
    //
    //  For new/delete overload.
    //
                  XilOpCopy() :
                      XilOpPoint(-1)
    {
    }

    _XIL_NEW_DELETE_OVERLOAD_PRIVATE(XilOpCopy)
};

#endif // _XIL_OP_COPY_HH
