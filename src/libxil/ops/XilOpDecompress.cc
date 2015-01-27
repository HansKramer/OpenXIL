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
//  File:        XilOpDecompress.cc
//  Project:     XIL
//  Revision:    1.19
//  Last Mod:    10:07:40, 03/10/00
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
//        COPYRIGHT
//------------------------------------------------------------------------
#pragma ident        "@(#)XilOpDecompress.cc	1.19\t00/03/10  "

#include <stdlib.h>
#include <string.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"
#include "XiliUtils.hh"

class XilOpDecompress : public XilOpPoint {
public:
    static XilOp* create(char* function_name, 
                         void* args[], 
                         int   count);

    Xil_boolean   isInPlace() const;
    Xil_boolean   canBeSplit();

    //    
    //  Over-riding setBoxStorage to ensure the source CIS has been sync'd
    //
    XilStatus     setBoxStorage(XiliRect*            rect,
                                XilDeferrableObject* object,
                                XilBox*              box);
    
    
protected:
    Xil_boolean   srcSyncd;
    XilCis*       source;

                  XilOpDecompress(XilOpNumber op_num);
                  ~XilOpDecompress();
};

//
//  Can't possily be in place, so don't bother to check
//
Xil_boolean
XilOpDecompress::isInPlace() const
{
    return FALSE;
}

//
//  We can't be split into multiple threads.
//
Xil_boolean
XilOpDecompress::canBeSplit()
{
    return FALSE;
}


//    
//  Over-riding setBoxStorage to ensure the source CIS has been sync'd
//
XilStatus
XilOpDecompress::setBoxStorage(XiliRect*            rect,
                               XilDeferrableObject* object,
                               XilBox*              box)
{
    if(object == source && !srcSyncd) {
        //
        //  We sync our source here because we're about to do the operation
        //  and we need to be certain that there are no outstanding compress
        //  operations about to write into our CIS that haven't been flushed.
        //
        if(source->sync() == XIL_FAILURE) {
            return XIL_FAILURE;
        }
        srcSyncd = TRUE;
    }

    *box = *rect;

    if(object->setBoxStorage(box) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

XilOp*
XilOpDecompress::create(char*  ,        // function_name
                        void* args[],
                        int   count)
{
    count = 0;                           // to remove comp. warning;

    XilCis*   cis   = (XilCis*)args[0];
    XilImage* image = (XilImage*)args[1];


    //
    // Verify that the dst image and the cis are valid and compatible
    // TODO: This could probably be moved to a utility function
    //
    if(image == NULL) {
        // NULL image specified
        XIL_ERROR(NULL, XIL_ERROR_USER, "di-207", TRUE);
        return NULL;
    }
    XilSystemState* state = image->getSystemState();

    if(!image->isValid()) {
        // Invalid source image
        XIL_ERROR(state, XIL_ERROR_USER, "di-327", TRUE);
        return NULL;
    }

    if(cis == NULL) {
        // NULL cis specified
        XIL_ERROR(state, XIL_ERROR_USER, "di-117", TRUE);
        return NULL;
    }

    if(cis->getReadInvalid()) {
        // Tried to read from an invalid cis
        XIL_ERROR(state, XIL_ERROR_USER, "di-224", TRUE);
        return NULL;
    }

    //
    // Verfify that the datatype of the
    // image is compatible with the cis
    //
    XilDeviceCompression* dc = cis->getDeviceCompression();

    unsigned int cis_w, cis_h, cis_b;
    unsigned int image_w, image_h, image_b;
    XilDataType image_t, cis_t;

    cis->getOutputType()->getInfo(&cis_w, &cis_h, &cis_b, &cis_t);
    image->getInfo(&image_w, &image_h, &image_b, &image_t);

    if(cis_w==0 || cis_h==0 || cis_b==0) {
        XIL_OBJ_ERROR(state, XIL_ERROR_SYSTEM, "di-294", TRUE, cis);
        return NULL;
    }

    if((cis_t != image_t) ||
       (cis_b != image_b) ||
       image_w == 0       ||
       image_h == 0) {
        XIL_OBJ_ERROR(state, XIL_ERROR_USER, "di-123", TRUE, cis);
        return NULL;
    }

    //
    //  Seek to the current location of the CIS.  The underlying CIS
    //  could have been moved by a deferred decompression or a
    //  deferred seek.
    //
    int read_frame  = cis->getReadFrame();
    int start_frame = cis->getStartFrame();
    int keep_frames = cis->getKeepFrames();
    dc->seek(read_frame, TRUE);

    //
    //  decompressHeader() is called to provide the compression with
    //  the ability to handle or setup any non-deferrable state which
    //  the user may request between this call and the actual
    //  occurance of the decompression.
    //
    if(dc->decompressHeader() == XIL_FAILURE) {
        XIL_ERROR(state, XIL_ERROR_SYSTEM, "di-418", TRUE);
        return NULL;
    }

    //
    // Get the op number for this type of decompression
    // Must be static to retain contents across calls
    // Arbitrarily make it a BIT datatype, so we can use
    // the XilOpcache
    //
    static XilOpCache op_cache;
    XilOpNumber       opnum;

    char buffer[1024];
    sprintf(buffer, "decompress_%s", dc->getDeviceManager()->getDeviceName());

    XilGlobalState*   xgs = XilGlobalState::getXilGlobalState();
    if((opnum = op_cache.set(XIL_BIT, xgs->lookupOpNumber(buffer))) < 0)
    {
        XIL_ERROR(state, XIL_ERROR_CONFIGURATION, "di-5", TRUE);
    }

    XilOpDecompress* op = new XilOpDecompress(opnum);
    if(op == NULL) {
        XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    op->setSrc(1, cis);
    op->setDst(1, image);

    //
    //  And stash the source away for our use in setBoxStorage().
    //
    op->source = cis;

    //
    // For deferred decompression, the argument to the decompression
    // also includes which frame in the CIS it should decompress.
    //
    op->setParam(1, read_frame);

    //
    // Advance read frame
    //
    read_frame++;
    cis->lock();
    cis->setReadFrame(read_frame);
    cis->unlock();

    //
    //  Determine if we must adjust our perception of the CIS starting
    //  frame number because keep_frames has been set.
    //
    if(keep_frames > 0 && (read_frame - start_frame) >= keep_frames) {
        start_frame = read_frame - keep_frames;
        cis->lock();
        cis->setStartFrame(start_frame);
        cis->unlock();

        cis->flushPriorDecompressOps(start_frame);
        //
        // Call the device compression to handle the removal of any
        // data associated with changing to the new start frame.
        //
        dc->adjustStart(start_frame);
    }


    return op;
}

XilOpDecompress::XilOpDecompress(XilOpNumber op_num) :
    XilOpPoint(op_num)
{
    srcSyncd = FALSE;
}

XilOpDecompress::~XilOpDecompress()
{
}
