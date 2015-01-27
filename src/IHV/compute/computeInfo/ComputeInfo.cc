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
//  File:	ComputeInfo.cc
//  Project:	XIL
//  Revision:	1.21
//  Last Mod:	10:13:32, 03/10/00
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
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)ComputeInfo.cc	1.21\t00/03/10  "

#include "ComputeInfo.hh"

ComputeInfo::ComputeInfo(XilOp*       init_op,
                         unsigned int init_op_count,
                         XilRoi*      init_roi,
                         XilBoxList*  init_bl) :
    src1Storage(init_op_count > 1 ?
                ((init_op->getOpList())[init_op_count-1])->getSrcImage(1) :
                init_op->getSrcImage(1)),
    src2Storage(init_op_count > 1 ?
                ((init_op->getOpList())[init_op_count-1])->getSrcImage(2) :
                init_op->getSrcImage(2)),
    src3Storage(init_op_count > 1 ?
                ((init_op->getOpList())[init_op_count-1])->getSrcImage(3) :
                init_op->getSrcImage(3)),
    destStorage(init_op->getDstImage(1)),
    src1BoxesOffset(0),
    src2BoxesOffset(1),
    src3BoxesOffset(2),
    rl(init_roi, NULL)
{
    //
    //  Set returnValue and isOKFlag to indicate things are so-far NOT ok.
    //
    returnValue = XIL_FAILURE;
    isOKFlag    = FALSE;

    op_count = init_op_count;
    roi      = init_roi;
    bl       = init_bl;

    //
    //  Molecule support.  dstOp is always what we're given.  srcOp is either
    //  the same as dstOp (no molecule) or the last one in the opList.
    //
    dstOp = init_op;
    if(op_count > 1) {
       srcOp = init_op->getOpList()[op_count-1];
    } else {
       srcOp = init_op;
    }

    //
    //  Initialize our varibles based on the number of images we have.  This
    //  way data elements which are not being used are never initialized, but
    //  that's ok since they should never be used.
    //
    bl->getNumBoxes(&numSrcs, &numDsts);

    //
    //  We need to initialize our images to NULL since they're expected to be
    //  NULL if they're not being used.
    //
    src1 = NULL;
    src2 = NULL;
    src3 = NULL;
    dest = NULL;
    
    switch(numSrcs) {
      case 3:
        src3               = srcOp->getSrcImage(3);
        src3NumBands       = src3->getNumBands();

      case 2:
        src2               = srcOp->getSrcImage(2);
        src2NumBands       = src2->getNumBands();

      case 1:
        src1               = srcOp->getSrcImage(1);
        src1NumBands       = src1->getNumBands();
    }
    
    switch(numDsts) {
      case 1:
        dest               = dstOp->getDstImage(1);
        destNumBands       = dest->getNumBands();
        destBoxesOffset    = numSrcs;

      case 0:
        break;

      default:
        return;
    }

    //
    //  If we have no images, then return because something is wrong.
    //
    if((src1 == NULL) && (dest == NULL)) {
        return;
    }

    //
    //  Initialize our counting flags, etc.
    //
    doneWithRectList     = TRUE;
    onNewBox             = FALSE;
    pixelSequentialFlag  = FALSE;
    bandSequentialFlag   = FALSE;
    boxCount             = 0;

    //
    //  If there are sources, split the box list based on the tiling in the
    //  sources.
    //
    if(numSrcs > 0) {
        if(dstOp->splitOnTileBoundaries(bl) == XIL_FAILURE) {
            return;
        }
    }

    //
    //  Set returnValue and isOKFlag to indicate things are so-far ok.
    //
    returnValue = XIL_SUCCESS;
    isOKFlag    = TRUE;
}

Xil_boolean
ComputeInfo::getNextBox()
{
    //
    //  If we've gotten this far, we're going to return TRUE and we consider
    //    that the routine has computed results for the first box.
    //
    if(onNewBox == TRUE) {
        boxCount++;
    }

    //
    //  We get the next box and then we fill in our XilStorage object with the
    //    next set of storage information for the new box.
    //
    if(bl->getNextArray(boxes) == FALSE) {
        return FALSE;
    }

    if(src3) {
        //
        //  Release any pre-existing storage...
        //
        src3Storage.release();

        //
        //  Aquire the storage...
        //
        if(src3->getStorage(&src3Storage, srcOp, boxes[src3BoxesOffset],
                            "XilMemory", XIL_READ_ONLY,
                            XIL_STORAGE_TYPE_UNDEFINED,
                            NULL) == XIL_FAILURE) {
            
            if(bl->markAsFailed() == XIL_FAILURE) {
                returnValue = XIL_FAILURE;
                return FALSE;
            } else {
                return getNextBox();
            }
        }

        //
        //  Aquire the storage information...
        //
        if(src3Storage.getStorageInfo(&src3PixelStride,
                                      &src3ScanlineStride,
                                      &src3BandStride,
                                      NULL,
                                      (void**)&src3BaseData) == XIL_FAILURE) {
            if(bl->markAsFailed() == XIL_FAILURE) {
                returnValue = XIL_FAILURE;
                return FALSE;
            } else {
                return getNextBox();
            }
        }
    }

    if(src2) {
        //
        //  Release any pre-existing storage...
        //
        src2Storage.release();

        if(src2->getStorage(&src2Storage, srcOp, boxes[src2BoxesOffset],
                            "XilMemory", XIL_READ_ONLY,
                            XIL_STORAGE_TYPE_UNDEFINED,
                            NULL) == XIL_FAILURE) { 
            if(bl->markAsFailed() == XIL_FAILURE) {
                returnValue = XIL_FAILURE;
                return FALSE;
            } else {
                return getNextBox();
            }
        }

        //
        //  Aquire the storage information...
        //
        if(src2Storage.getStorageInfo(&src2PixelStride,
                                      &src2ScanlineStride,
                                      &src2BandStride,
                                      NULL,
                                      (void**)&src2BaseData) == XIL_FAILURE) {
            if(bl->markAsFailed() == XIL_FAILURE) {
                returnValue = XIL_FAILURE;
                return FALSE;
            } else {
                return getNextBox();
            }
        }
    }

    if(src1) {
        //
        //  Release any pre-existing storage...
        //
        src1Storage.release();

        if(src1->getStorage(&src1Storage, srcOp, boxes[src1BoxesOffset],
                            "XilMemory", XIL_READ_ONLY,
                            XIL_STORAGE_TYPE_UNDEFINED,
                            NULL) == XIL_FAILURE) { 
            if(bl->markAsFailed() == XIL_FAILURE) {
                returnValue = XIL_FAILURE;
                return FALSE;
            } else {
                return getNextBox();
            }
        }

        //
        //  Aquire the storage information...
        //
        if(src1Storage.getStorageInfo(&src1PixelStride,
                                      &src1ScanlineStride,
                                      &src1BandStride,
                                      NULL,
                                      (void**)&src1BaseData) == XIL_FAILURE) {
            if(bl->markAsFailed() == XIL_FAILURE) {
                returnValue = XIL_FAILURE;
                return FALSE;
            } else {
                return getNextBox();
            }
        }
    }

    //
    //  If we have a destination, get its storage.
    //
    if(dest) {
        //
        //  Release any pre-existing storage...
        //
        destStorage.release();

        if(dest->getStorage(&destStorage, dstOp, boxes[destBoxesOffset],
                            "XilMemory", XIL_WRITE_ONLY,
                             XIL_STORAGE_TYPE_UNDEFINED,
                             NULL) == XIL_FAILURE) { 
            if(bl->markAsFailed() == XIL_FAILURE) {
                returnValue = XIL_FAILURE;
                return FALSE;
            } else {
                return getNextBox();
            }
        }

        if(destStorage.getStorageInfo(&destPixelStride,
                                      &destScanlineStride,
                                      &destBandStride,
                                      NULL,
                                      (void**)&destBaseData) == XIL_FAILURE) {
            if(bl->markAsFailed() == XIL_FAILURE) {
                returnValue = XIL_FAILURE;
                return FALSE;
            } else {
                return getNextBox();
            }
        }
    }

    //
    //  Re-initialize the XilRectList with the new Box.  This uses the same
    //    ROI from before, but produces a list that has been interesected with
    //    the new XilBox instead of the previous XilBox.
    //
    if(dest == NULL) {
        rl.reinit(boxes[src1BoxesOffset]);
    } else {
        rl.reinit(boxes[destBoxesOffset]);
    }

    return TRUE;
}

Xil_boolean
ComputeInfo::hasMoreInfo()
{
    if(doneWithRectList == TRUE) {
        if(getNextBox() == FALSE) { // we're done
            return FALSE;
        }
    }
    
    //
    //  This is a while loop to handle the case of a rect list not having
    //    rectangles in it.
    //
    while(rl.getNext(&x, &y, &xsize, &ysize) == FALSE) {
        doneWithRectList = TRUE;

        if(getNextBox() == FALSE) { // we're done
            return FALSE;
        }
    }

    //
    //  We call a pure-virtual function that is implemented by the derived
    //    classes to fill in their data fields at this point.
    //
    if(updateForNewRect() == XIL_FAILURE) {
        if(bl->markAsFailed() == XIL_FAILURE) {
            returnValue = XIL_FAILURE;
            return FALSE;
        } else {
            return getNextBox();
        }
    }
    
    //
    //  The doneWithRectList flag is set to indicate that the hasMoreInfo
    //    routine has extracted all of the information from the XilRectList
    //    and it it time to move on to a new XilBox.
    //
    doneWithRectList = FALSE;

    return TRUE;
}

Xil_boolean
ComputeInfo::isStorageType(XilStorageType target_type)
{
    switch(numSrcs) {
      case 3:
        if(src3Storage.isType(target_type) == FALSE) {
            return FALSE;
        }

      case 2:
        if(src2Storage.isType(target_type) == FALSE) {
            return FALSE;
        }

      case 1:
        if(src1Storage.isType(target_type) == FALSE) {
            return FALSE;
        }
    }

    if(dest) {
        if(destStorage.isType(target_type) == FALSE) {
            return FALSE;
        }
    }        

    return TRUE;
}
