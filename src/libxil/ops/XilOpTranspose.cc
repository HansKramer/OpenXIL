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
//  File:	XilOpTranspose.cc
//  Project:	XIL
//  Revision:	1.36
//  Last Mod:	10:07:28, 03/10/00
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
#pragma ident	"@(#)XilOpTranspose.cc	1.36\t00/03/10  "

#include <stdlib.h>
#include <xil/xilGPI.hh>
#include "XilOpGeometric.hh"
#include "XilOpTranspose.hh"
#include "XiliOpUtils.hh"

XilOp*
XilOpTranspose::create(char  function_name[],
                  void* args[],
                  int)
{
    static XilOpCache transpose_op_cache;
    XilImage*         src = (XilImage*)args[0];
    XilImage*         dst = (XilImage*)args[1];
    
    XilOpNumber opnum;
    if((opnum = xili_verify_op_args(function_name, &transpose_op_cache,
                                    dst, src)) == -1) {
        return NULL;
    }

    //
    //  Get the flip type
    //
    XilFlipType flip = ((XilFlipType)((int)args[2]));

    XilOpTranspose* op = new XilOpTranspose(opnum, flip,
                                            src->getWidth(),
                                            src->getHeight());
    if(op == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }
  
    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, flip);

    dst->setPixelWidth(src->getPixelWidth());
    dst->setPixelHeight(src->getPixelHeight());

    return op;
}

//
// This is overloaded because we need to do the roi
// intersection accounting for transpose type. Transpose
// also ignores image origins hence we need to get the
// unadjusted roi.
//
XilStatus
XilOpTranspose::generateIntersectedRoi()
{
    XilImage*    source = getSrcImage(1);
    XilImage*    dest   = getDstImage(1);
    
    //
    // Get the rois we need
    //
    XilRoi*      transposed_roi;
    XilRoi*      sgs_roi = getSrcGlobalSpaceRoi(0);
    XilRoi*      dgs_roi = getDstGlobalSpaceRoi(0);
    XilRoi*      src_roi = (XilRoi*)sgs_roi->createCopy();
    XilRoi*      dst_roi = (XilRoi*)dgs_roi->createCopy();
    XilRoi*      intersected_roi = getIntersectedRoi();

    //
    // assign a system state to the intersectedRoi since it was
    // constructed without one. First check to see if the intersectedRoi
    // is valid.
    //
    if(intersected_roi == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    // Reset the Global space Rois we copied from the image
    // to be in image space. Transpose ignores image origins.
    //
    source->convertToObjectSpace(src_roi);
    dest->convertToObjectSpace(dst_roi);

    float xo = 0.0;
    float yo = 0.0;
    switch(fliptype) {
      case XIL_FLIP_X_AXIS:
      case XIL_FLIP_Y_AXIS:
      case XIL_FLIP_180:
        xo = ((float) src_xsize - 1.0F) / 2.0F;
        yo = ((float) src_ysize - 1.0F) / 2.0F;
        break;
        
      case XIL_FLIP_MAIN_DIAGONAL:
        break;
        
      case XIL_FLIP_ANTIDIAGONAL:
        src_roi->translate_inplace((float)(1.0F-(float)src_xsize),
                                   (float)(1.0F-(float)src_ysize));
        break;
                    
      case XIL_FLIP_90:
        src_roi->translate_inplace((float)(1.0F-(float)src_xsize), 0.0F);
        break;
        
      case XIL_FLIP_270:
        src_roi->translate_inplace(0.0F, (1.0F-(float)src_ysize));
        break;
    }
    
    transposed_roi = src_roi->transpose(fliptype, xo, yo);
    dst_roi->intersect(transposed_roi, intersected_roi);
    transposed_roi->destroy();
    src_roi->destroy();
    dst_roi->destroy();

    return XIL_SUCCESS;
}    

//
//  Move between global and object space....
//
//
//  Transpose ignores image origins so we just return without requesting
//  the object to translate the rect or ROI.
//
XilStatus
XilOpTranspose::moveIntoObjectSpace(XiliRect*            ,
                                    XilDeferrableObject* )
{
    return XIL_SUCCESS;
}

XilStatus
XilOpTranspose::moveIntoObjectSpace(XilRoi*              ,
                                    XilDeferrableObject* )
{
    return XIL_SUCCESS;
}

XilStatus
XilOpTranspose::moveIntoGlobalSpace(XiliRect*            ,
                                    XilDeferrableObject* )
{
    return XIL_SUCCESS;
}

XilStatus
XilOpTranspose::moveIntoGlobalSpace(XilRoi*              ,
                                    XilDeferrableObject* )
{
    return XIL_SUCCESS;
}
    
//
//  In this implementation of divideBoxList we split the source box and then
//  forward map it to get the corresponding destination box. This maintains
//  the box to box mapping that was established in the initial generation of
//  the box list. 
//
Xil_boolean
XilOpTranspose::divideBoxList(XilBoxList*   boxlist,
                              unsigned int  box_number,
                              unsigned int  tile_xdelta,
                              unsigned int  tile_ydelta)
{
    //
    //  "real" corners for box list entry.
    //
    int box_x1, box_x2, box_y1, box_y2;

    //
    //  storage corners for box list entry.
    //
    int box_x1s, box_x2s, box_y1s, box_y2s, box_band;

    //
    //  "real" corners for active box.
    //
    int abox_x1, abox_x2, abox_y1, abox_y2;

    //
    //  storage corners for active box.
    //
    int abox_x1s, abox_x2s, abox_y1s, abox_y2s, abox_band;

    //
    // Dest image
    //
    XilImage*    dest = getDstImage(1);

    //
    //  TODO: 12/18/95 maynard  if(box_number > ble->boxCount) return FALSE;
    //
    //        Not checking initial argument, because I don't want to do
    //        it in the loop.
    //
    XiliSLList<XiliBoxListEntry*>*        list = boxlist->getList();
    XiliSLListIterator<XiliBoxListEntry*> bl_iterator(list);
    XiliBoxListEntry*                     ble;
    while(bl_iterator.getNext(ble) == XIL_SUCCESS) {
        //
        //  Get the corresponding box from the entry we're going to split.
        //
        XilBox* active_box = &ble->boxes[box_number];
        
        //
        //  Get both the "real" corners and the storage corners.  We're doing
        //  the splitting based on the storage corners since the tile boundaries
        //  are with respect to the storage. 
        //
        active_box->getAsCorners(&abox_x1, &abox_y1,
                                 &abox_x2, &abox_y2);
        active_box->getStorageAsCorners(&abox_x1s, &abox_y1s,
                                        &abox_x2s, &abox_y2s, &abox_band);

#ifdef BOX_DEBUG        
        fprintf(stderr, "\nActive box while ble loop\n");
        active_box->dump();
        ble->boxes[1].dump();
#endif        

        //
        //  Does a tile boundary split this box in X?
        //
        unsigned int tile_count     = abox_x1s/tile_xdelta;
        unsigned int tile_xboundary = (tile_xdelta*(tile_count+1) - 1);

        if((unsigned int) abox_x2s > tile_xboundary) {
            //
            //  Split on X-boundary
            //
            unsigned int      delta_x = tile_xboundary - abox_x1s;

            XiliBoxListEntry* new_ble = new XiliBoxListEntry;

            new_ble->boxCount = ble->boxCount;

            //
            // Get the pixel and storage co-ordinates
            //
#ifdef BOX_DEBUG                
            fprintf(stderr, "\nOriginal Boxes before forward mapped X split\n");
            ble->boxes[0].dump();
            ble->boxes[1].dump();
#endif
                
            ble->boxes[0].getAsCorners(&box_x1, &box_y1, &box_x2, &box_y2);
            ble->boxes[0].getStorageAsCorners(&box_x1s, &box_y1s,
                                              &box_x2s, &box_y2s,
                                              &box_band);

            //
            //  Split the box into two, for storage and pixels
            //
            ble->boxes[0].setAsCorners(box_x1,
                                       box_y1,
                                       box_x1 + delta_x,
                                       box_y2);
            ble->boxes[0].setStorageAsCorners(box_x1s,
                                              box_y1s,
                                              box_x1s + delta_x,
                                              box_y2s,
                                              box_band);

            new_ble->boxes[0].setAsCorners(box_x1 + delta_x + 1,
                                           box_y1,
                                           box_x2,
                                           box_y2);
            new_ble->boxes[0].setStorageAsCorners(box_x1s + delta_x + 1,
                                                  box_y1s,
                                                  box_x2s,
                                                  box_y2s,
                                                  box_band);
            //
            //  Now to get the destination storage forward map the source box
            //  into the destination, and set the storage.  We re-calculate
            //  the storage based on the pixel co-ordinates as we can't map
            //  storage co-ordinates in all cases eg, child images. We use
            //  num_srcs to tell us where dest box is.
            //
            XiliRectInt tmp_rect(box_x1, box_y1, box_x1 + delta_x, box_y2);
            gsForwardMap(&tmp_rect, 1, &tmp_rect);
            ble->boxes[1] = tmp_rect;
            dest->setBoxStorage(&ble->boxes[1]);

            tmp_rect.set(box_x1 + delta_x + 1, box_y1, box_x2, box_y2);
            gsForwardMap(&tmp_rect, 1, &tmp_rect);
            new_ble->boxes[1] = tmp_rect;
            dest->setBoxStorage(&new_ble->boxes[1]);

#ifdef BOX_DEBUG                
            fprintf(stderr, "\nOriginal boxes after forward mapped X split\n");
            ble->boxes[0].dump();
            ble->boxes[1].dump();
                    
            fprintf(stderr, "\nNew boxes after forward mapped X split\n");
            new_ble->boxes[0].dump();
            new_ble->boxes[1].dump();
#endif                

            if(list->insertAfter(new_ble,
                                 bl_iterator.getCurrentPosition()) == _XILI_SLLIST_INVALID_POSITION) {
                //
                //  Oddly enough, insertion failed.
                //
                //  TODO: 2/26/96 jlf  Generate secondary failure?
                //
                return FALSE;
            }
        }

        //
        //  Does a tile boundary split this box in Y?
        //
        unsigned int tile_yboundary;

        tile_count     = abox_y1s/tile_ydelta;
        tile_yboundary = (tile_ydelta*(tile_count+1) -1);

        while((unsigned int) abox_y2s > tile_yboundary) {
            //
            //  Split the created box along possible y's
            //
            unsigned int      delta_y = tile_yboundary - abox_y1s;

            XiliBoxListEntry* new_ble = new XiliBoxListEntry;
          
            new_ble->boxCount = ble->boxCount;

            //
            // Get the pixel and storage co-ordinates
            //
#ifdef BOX_DEBUG                
            fprintf(stderr, "\nOriginal Boxes before forward mapped Y split\n");
            ble->boxes[0].dump();
            ble->boxes[1].dump();
#endif                

            ble->boxes[0].getAsCorners(&box_x1,&box_y1,&box_x2,&box_y2);
            ble->boxes[0].getStorageAsCorners(&box_x1s, &box_y1s,
                                              &box_x2s, &box_y2s,
                                              &box_band);

            //
            // Split the box into two, for storage and pixels
            //
            ble->boxes[0].setAsCorners(box_x1,
                                       box_y1,
                                       box_x2,
                                       box_y1 + delta_y);
            ble->boxes[0].setStorageAsCorners(box_x1s,
                                              box_y1s,
                                              box_x2s,
                                              box_y1s + delta_y,
                                              box_band);

            new_ble->boxes[0].setAsCorners(box_x1,
                                           box_y1 + delta_y + 1,
                                           box_x2,
                                           box_y2);
            new_ble->boxes[0].setStorageAsCorners(box_x1s,
                                                  box_y1s + delta_y + 1,
                                                  box_x2s,
                                                  box_y2s,
                                                  box_band);

            //
            //  Now to get the destination storage forward map the source box
            //  into the destination, and set the storage.  We re-calculate
            //  the storage based on the pixel co-ordinates as we can't map
            //  storage co-ordinates in all cases eg, child images. We use
            //  num_srcs to tell us where dest box is.
            //
            XiliRectInt tmp_rect(box_x1, box_y1, box_x2, box_y1 + delta_y);
            gsForwardMap(&tmp_rect, 1, &tmp_rect);
            ble->boxes[1] = tmp_rect;
            dest->setBoxStorage(&ble->boxes[1]);

            tmp_rect.set(box_x1, box_y1 + delta_y + 1, box_x2, box_y2);
            gsForwardMap(&tmp_rect, 1, &tmp_rect);
            new_ble->boxes[1] = tmp_rect;
            dest->setBoxStorage(&new_ble->boxes[1]);

#ifdef BOX_DEBUG                
            fprintf(stderr, "\nOriginal boxes after forward mapped Y split\n");
            ble->boxes[0].dump();
            ble->boxes[1].dump();
                    
            fprintf(stderr, "\nNew boxes after forward mapped Y split\n");
            new_ble->boxes[0].dump();
            new_ble->boxes[1].dump();
#endif                

            if(list->insertAfter(new_ble,
                                bl_iterator.getCurrentPosition()) == _XILI_SLLIST_INVALID_POSITION) {
                //
                //  Oddly enough, insertion failed.
                //
                //  TODO: 2/26/96 jlf  Generate secondary failure?
                //
                return FALSE;
            }

            //
            //  Move to the next tile boundary in y
            //
            tile_yboundary += tile_ydelta;

            //
            //  Move on to the next box which will be the latter (bottom)
            //  portion of our most recent split.  This should always work if
            //  insertAfter() worked.  If it doesn't then we've got a bigger
            //  problem.
            //
            if(bl_iterator.getNext(ble) == XIL_FAILURE) {
                //
                //  TODO: 2/26/96 jlf  Generate secondary failure?
                //
                return FALSE;
            }

            //
            //  Reset our active box to the next one in the list.
            //
            active_box = &ble->boxes[box_number];
            
            active_box->getAsCorners(&abox_x1, &abox_y1, &abox_x2, &abox_y2);
            active_box->getStorageAsCorners(&abox_x1s, &abox_y1s,
                                            &abox_x2s, &abox_y2s,
                                            &abox_band);
        }
    }

    return TRUE;
}

//
//  Overload backward map as we need to change the source rect based on the
//  flip type. 
//
XilStatus
XilOpTranspose::gsBackwardMap(XiliRect*    dst_rect,
                              XiliRect*    src_rect,
                              unsigned int )
{
    int x1;
    int y1;
    int x2;
    int y2;
    dst_rect->get(&x1, &y1, &x2, &y2);

    //
    //  Boxes must have x2 > x1 and y2 > y1 therefore when we do the mapping
    //  we also adjust the co-ordinates so that the source box is always
    //  valid.
    //
    int sx1;
    int sy1;
    int sx2;
    int sy2;
    switch(fliptype) {
      case XIL_FLIP_X_AXIS:
        //
        //  Need to swap y values
        //
        sx1 = x1;
        sx2 = x2;
        sy1 = src_ysize - y2 - 1;
        sy2 = src_ysize - y1 - 1;
        break;
        
      case XIL_FLIP_Y_AXIS:
        //
        //  Need to swap x values
        //
        sx1 = src_xsize - x2 - 1;
        sx2 = src_xsize - x1 - 1;
        sy1 = y1;
        sy2 = y2;
        break;
        
      case XIL_FLIP_MAIN_DIAGONAL:
        //
        //  No need to swap positions
        //
        sx1 = y1;
        sx2 = y2;
        sy1 = x1;
        sy2 = x2;
        break;
        
      case XIL_FLIP_ANTIDIAGONAL:
        //
        //  Swap positions for x and y
        //
        sx1 = src_xsize - y2 - 1;
        sx2 = src_xsize - y1 - 1;
        sy1 = src_ysize - x2 - 1;
        sy2 = src_ysize - x1 - 1;
        break;
        
      case XIL_FLIP_90:
        //
        //  Swap x values
        //
        sx1 = src_xsize - y2 - 1;
        sx2 = src_xsize - y1 - 1;
        sy1 = x1;
        sy2 = x2;
        break;
        
      case XIL_FLIP_180:
        //
        //  Swap x and y values
        //
        sx1 = src_xsize - x2 - 1;
        sx2 = src_xsize - x1 - 1;
        sy1 = src_ysize - y2 - 1;
        sy2 = src_ysize - y1 - 1;
        break;
        
      case XIL_FLIP_270:
        //
        //  Swap y values
        //
        sx1 = y1;
        sx2 = y2;
        sy1 = src_ysize - x2 - 1;
        sy2 = src_ysize - x1 - 1;
        break;
    }
    
    src_rect->set(sx1, sy1, sx2, sy2);

    return XIL_SUCCESS;
}
        
//
//  Overload forward map as we need to change the destination rect based on
//  the flip type. 
//
XilStatus
XilOpTranspose::gsForwardMap(XiliRect*    src_rect,
                             unsigned int ,
                             XiliRect*    dst_rect)
{
    int x1;
    int y1;
    int x2;
    int y2;
    src_rect->get(&x1, &y1, &x2, &y2);

    int dx1;
    int dy1;
    int dx2;
    int dy2;
    switch(fliptype) {
      case XIL_FLIP_X_AXIS:
        //
        //  Need to swap y values
        //
        dx1 = x1;
        dx2 = x2;
        dy1 = src_ysize - y2 - 1;
        dy2 = src_ysize - y1 - 1;
        break;
        
      case XIL_FLIP_Y_AXIS:
        //
        //  Need to swap x values
        //
        dx1 = src_xsize - x2 - 1;
        dx2 = src_xsize - x1 - 1;
        dy1 = y1;
        dy2 = y2;
        break;
        
      case XIL_FLIP_MAIN_DIAGONAL:
        //
        //  No need to swap positions
        //
        dx1 = y1;
        dx2 = y2;
        dy1 = x1;
        dy2 = x2;
        break;

      case XIL_FLIP_ANTIDIAGONAL:
        //
        //  Swap positions for x and y
        //
        dx1 = src_ysize - y2 - 1;
        dx2 = src_ysize - y1 - 1;
        dy1 = src_xsize - x2 - 1;
        dy2 = src_xsize - x1 - 1;
        break;
        
      case XIL_FLIP_90:
        //
        //  Swap y values
        //
        dx1 = y1;
        dx2 = y2;
        dy1 = src_xsize - x2 - 1;
        dy2 = src_xsize - x1 - 1;
        break;

      case XIL_FLIP_180:
        //
        //  Swap x and y values
        //
        dx1 = src_xsize - x2 - 1;
        dx2 = src_xsize - x1 - 1; 
        dy1 = src_ysize - y2 - 1;
        dy2 = src_ysize - y1 - 1;
        break;

      case XIL_FLIP_270:
        //
        //  Swap y values
        //
        dx1 = src_ysize - y2 - 1;
        dx2 = src_ysize - y1 - 1;
        dy1 = x1;
        dy2 = x2;
        break;
    }
    
    dst_rect->set(dx1, dy1, dx2, dy2);

    return XIL_SUCCESS;
}

XilStatus
XilOpTranspose::vBackwardMap(XilBox*       dst_box,
                             double        dx,
                             double        dy,
                             XilBox*       src_box,
                             double*       sx,
                             double*       sy,
                             unsigned int  )
{
    //
    //  Move the rect into image co-ordinates, from the box.
    //
    int          dbox_x, dbox_y;
    unsigned int dbox_w, dbox_h;
    dst_box->getAsRect(&dbox_x, &dbox_y, &dbox_w, &dbox_h);

    int          sbox_x, sbox_y;
    unsigned int sbox_w, sbox_h;
    src_box->getAsRect(&sbox_x, &sbox_y, &sbox_w, &sbox_h);

    //
    //  Add in the rect values
    //
    dbox_x += _XILI_ROUND(dx);
    dbox_y += _XILI_ROUND(dy);

    switch (fliptype) {
      case XIL_FLIP_X_AXIS:
        *sx = dbox_x;
        *sy = src_ysize - dbox_y - 1;
        break;

      case XIL_FLIP_Y_AXIS:
        *sx = src_xsize - dbox_x - 1;
        *sy = dbox_y;
        break;

      case XIL_FLIP_MAIN_DIAGONAL:
        *sx = dbox_y;
        *sy = dbox_x;
        break;

      case XIL_FLIP_ANTIDIAGONAL:
        *sx = src_xsize - dbox_y - 1;
        *sy = src_ysize - dbox_x - 1;
        break;

      case XIL_FLIP_90:
        *sx = src_xsize - dbox_y - 1;
        *sy = dbox_x;
        break;

      case XIL_FLIP_180:
        *sx = src_xsize - dbox_x - 1;
        *sy = src_ysize - dbox_y - 1;
        break;

      case XIL_FLIP_270:
        *sx = dbox_y;
        *sy = src_ysize - dbox_x - 1;
        break;
    }

    //
    // Reset the source rect to box space
    //
    *sx -= sbox_x;
    *sy -= sbox_y;

    return XIL_SUCCESS;
}
