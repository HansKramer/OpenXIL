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
//  File:	XilOpCopy.cc
//  Project:	XIL
//  Revision:	1.16
//  Last Mod:	10:06:56, 03/10/00
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
#pragma ident	"@(#)XilOpCopy.cc	1.16\t00/03/10  "

#include <stdlib.h>

#include <xil/xilGPI.hh>
#include "XilOpCopy.hh"
#include "XiliOpUtils.hh"

#include <stdio.h>
//
//  This constructs them one at a time up to a maximum of 16 existing at any
//  one time.  We don't expect to need more than this number of copy
//  operations deferred at the same moment very often.
//
_XIL_NEW_DELETE_OVERLOAD_CC_FILE_1(XilOpCopy, 8)

XilOp*
XilOpCopy::create(char  function_name[],
                  void* args[],
                  int)
{
    XilImage* src = (XilImage*)args[0];
    XilImage* dst = (XilImage*)args[1];
    
    static XilOpCache  copy_op_cache;
    
    XilOpNumber opnum;
    if((opnum = xili_verify_op_args(function_name, &copy_op_cache,
                                    dst, src))== -1) {
        return NULL;
    }

    XilOpCopy* op = new XilOpCopy(opnum);

    op->setSrc(1, src);
    op->setDst(1, dst);

#ifdef XILI_COPY_REORDERING_IMPLEMENTED
    //
    //  Check to see if this is an operation which may require tile
    //  reordering...
    //
    XilImage* src_parent = src->getParent();
    XilImage* dst_parent = dst->getParent();
    if(src_parent == dst_parent) {
        op->setReordersTiles(TRUE);
        op->reorderingSet = FALSE;
    }
#endif

    //
    //  Check to see if this is an operation which may require tile
    //  reordering...
    //
    XilImage* src_parent = src->getParent();
    XilImage* dst_parent = dst->getParent();
    if(src_parent != NULL || dst_parent != NULL) {
        if(src_parent == dst_parent ||
           src        == dst_parent ||
           src_parent == dst) {
            op->copyCanBeSplit = FALSE;
        } else {
            op->copyCanBeSplit = TRUE;
        }            
    } else {
        op->copyCanBeSplit = TRUE;
    }

    dst->setPixelWidth(src->getPixelWidth());
    dst->setPixelHeight(src->getPixelHeight());

    return op;
}

Xil_boolean
XilOpCopy::canBeSplit()
{
    return copyCanBeSplit;
}

XilStatus
XilOpCopy::vSplitOnTileBoundaries(XilBoxList* box_list)
{
    if(copyCanBeSplit) {
        return XilOp::vSplitOnTileBoundaries(box_list);
    } else {
        //
        //  If we can't be split, then we can't split on tile boundaries in
        //  the source because that affects the destination boxes too.
        //
        return XIL_SUCCESS;
    }
}


#ifdef XILI_COPY_REORDERING_IMPLEMENTED
//
//  Permits the op to reorder the processing of tiles by the core.  This
//  is only called if reorderTiles is set.
//
XilStatus
XilOpCopy::reorderTileProcessing(XilTileList* old_list,
                                 XilTileList* new_list)
{
    //
    //  We know our source and destinations are children of the same parent.
    //  Now, we check to see if they intersect and if they do, we may reorder
    //  the tile processing in the destination.
    //
    if(! reorderingSet) {
        XilImage* src = getSrcImage(1);
        XilImage* dst = getDstImage(1);

        //
        //  Does the destination image span tiles?
        //
        if(dst->getNumTiles() == 1) {
            //
            //  Only one tile, so we don't need to worry about reordering the
            //  tile list.
            //
            new_list->setEqual(old_list);
            setReordersTiles(FALSE);

            return XIL_SUCCESS;
        }

        unsigned int txsize;
        unsigned int tysize;
        dst->getTileSize(&txsize, &tysize);

        unsigned int xoff;
        unsigned int yoff;
        unsigned int boff;
        src->getChildOffsets(&xoff, &yoff, &boff);

        unsigned int width = src->getWidth();
        unsigned int height = src->getHeight();

        int src_x1 = xoff;
        int src_y1 = yoff;
        int src_x2 = xoff + width - 1;
        int src_y2 = yoff + height - 1;

        dst->getChildOffsets(&xoff, &yoff, &boff);
        width  = dst->getWidth();
        height = dst->getHeight();

        int dst_x1 = xoff;
        int dst_y1 = yoff;
        int dst_x2 = xoff + width - 1;
        int dst_y2 = yoff + height - 1;

        //
        //  Ok, does the destination span tiles?
        //
        if((dst_x1 / txsize) == (dst_x2 / txsize) &&
           (dst_y1 / tysize) == (dst_y2 / tysize)) {
            //
            //  Nope, the corners fall into the same tile.
            //
            new_list->setEqual(old_list);
            setReordersTiles(FALSE);

            return XIL_SUCCESS;
        }            

        int tmp_x1 = _XILI_MAX(src_x1, dst_x1);
        int tmp_x2 = _XILI_MIN(src_x2, dst_x2);
        int tmp_y1 = _XILI_MAX(src_y1, dst_y1);
        int tmp_y2 = _XILI_MIN(src_y2, dst_y2);

        //
        //  If it's clipped to an empty rect, then we don't need to reorder!
        //
        if((tmp_x2 < tmp_x1) || (tmp_y2 < tmp_y1)) {
            new_list->setEqual(old_list);
            setReordersTiles(FALSE);

            return XIL_SUCCESS;
        }

        fprintf(stderr, "We need to reorder tiles!\n");

        if(dst_x1 > src_x1 && dst_x1 < src_x2) {
            //
            //  Dst starts inside of source which means we need
            //    right->left AND bottom->top
            //
            fprintf(stderr, "right->left AND bottom->top\n");
            reorderDir = XILI_COPY_ORDER_R2L_B2T;

            new_list->setEqual(old_list);

            unsigned int num_tiles = old_list->getNumTiles();
            for(int i=0; i<num_tiles; i++) {
                XilTileNumber old_tile =
                    old_list->getTileNumber(num_tiles - i - 1);

                new_list->setEntry(i, old_tile);
            }
        } else if(dst_x1 < src_x1 && dst_y1 < src_y1) {
            //
            //  Dst starts to the left and higher than source which means we need
            //    left->right AND top->bottom
            //
            fprintf(stderr, "left->right AND top->bottom\n");

            new_list->setEqual(old_list);
            setReordersTiles(FALSE);

            return XIL_SUCCESS;
        } else if(dst_x1 < src_x1 && dst_y1 > src_y1) {
            //
            //  Dst starts to the left yet inside of source which means we need
            //    left->right AND bottom->top
            //
            fprintf(stderr, "right->left AND bottom->top\n");
            reorderDir = XILI_COPY_ORDER_L2R_B2T;
        } else if(dst_x1 > src_x1 && dst_y1 < src_y1) {
            //
            //  Dst starts inside yet above source which means we need
            //    right->left AND top->bottom
            //
            fprintf(stderr, "right->left AND top->bottom\n");
            reorderDir = XILI_COPY_ORDER_R2L_B2T;
        }            
    }

    return XIL_SUCCESS;
}
#endif
