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
//  File:	XilOpCompress.cc
//  Project:	XIL
//  Revision:	1.21
//  Last Mod:	10:07:48, 03/10/00
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
#pragma ident	"@(#)XilOpCompress.cc	1.21\t00/03/10  "

#include <stdlib.h>
#include <string.h>

#include <xil/xilGPI.hh>
#include "XilOpPoint.hh"
#include "XiliOpUtils.hh"
#include "XiliUtils.hh"

class XilOpCompress : public XilOpPoint {
public:
    static XilOp* create(char*  function_name, 
                         void* args[], 
                         int   count);

    Xil_boolean flushOnInsert() const;
    virtual Xil_boolean thisOpCoversPreviousOp();
    Xil_boolean isInPlace() const;
    Xil_boolean canBeSplit();


protected:
    XilOpCompress(XilOpNumber op_num);
    ~XilOpCompress();

};

//
// Returns FALSE for deferred execution purposes
//
Xil_boolean
XilOpCompress::flushOnInsert() const
{
    return FALSE;
}

//
// Returns FALSE for deferred execution purposes.
// Due to the fact that the default implementation
// modifies the Op queue length which should not be
// the case for cis ops.
//
Xil_boolean
XilOpCompress::thisOpCoversPreviousOp()
{
    return FALSE;
}

//
// Can't possily be in place, so don't bother to check
//
Xil_boolean
XilOpCompress::isInPlace() const
{
    return FALSE;
}

//
//  We can't be split into multiple threads.
//
Xil_boolean
XilOpCompress::canBeSplit()
{
    return FALSE;
}

XilOp*
XilOpCompress::create(char* ,        // function_name
                      void* args[],
                      int   count)
{
    count = 0;                               // to remove comp. warning

    XilImage* src_image = (XilImage*)args[0];
    XilCis*   cis       = (XilCis*)args[1];

    //
    // Verify that the src image and the cis are valid and compatible
    //
    if (src_image == NULL) {
        // NULL image specified
        XIL_ERROR(NULL, XIL_ERROR_USER, "di-207", TRUE);
        return NULL;
    }
    XilSystemState* state = src_image->getSystemState();

    if(!src_image->isValid()) {
        // Invalid source image
        XIL_ERROR(state, XIL_ERROR_USER, "di-327", TRUE);
        return NULL;
    }

    if (cis == NULL) {
        // NULL cis specified
        XIL_ERROR(state, XIL_ERROR_USER, "di-117", TRUE);
        return NULL;
    }

    if(cis->getWriteInvalid()) {
        // Tried to write to an invalid cis
        XIL_ERROR(state, XIL_ERROR_USER, "di-223", TRUE);
        return NULL;
    }

    //
    // Verfify that the size, datatype and band count of the
    // image is compatible with the cis
    //
    XilDeviceCompression* dc = cis->getDeviceCompression();

    unsigned int cis_w, cis_h, cis_b;
    unsigned int image_w, image_h, image_b;
    XilDataType image_t, cis_t;

    dc->getInputType()->getInfo(&cis_w, &cis_h, &cis_b, &cis_t);
    src_image->getInfo(&image_w, &image_h, &image_b, &image_t);
    if(cis_w != image_w || cis_h != image_h ||
       cis_b != image_b || cis_t != image_t ||
       cis_w == 0 || cis_h == 0 || image_w == 0 || image_h == 0) {
        if(dc->setInputType(src_image) != XIL_SUCCESS) {
            XIL_ERROR(state, XIL_ERROR_INTERNAL, "di-417", TRUE);
            return NULL;
        }
    }

    //
    //  Verify that the image does not have any ROIs or a non-zero origin.
    //  The problem here is that there is really no way for ROI or
    //  origin information to persist once the image is compressed 
    //  into the CIS. On decompression, we would have no way of knowing
    //  that the frame had these properties.
    //
    if(src_image->getRoi()     != NULL || 
       src_image->getOriginX() != 0.0  ||
       src_image->getOriginY() != 0.0) {
        XIL_OBJ_ERROR(state, XIL_ERROR_USER, "di-288", TRUE, src_image);
        return NULL;
    }

    //
    // Get the op number for this type of compression
    // Must be static to retain contents across calls
    // Arbitrarily make it a BIT datatype, so we can use
    // the XilOpCache
    //
    static XilOpCache op_cache;
    XilOpNumber       op_number;

    char buffer[1024];
    sprintf(buffer, "compress_%s", dc->getDeviceManager()->getDeviceName());

    XilGlobalState*   xgs = XilGlobalState::getXilGlobalState();
    if((op_number = op_cache.set(XIL_BIT, xgs->lookupOpNumber(buffer))) < 0) {
        XIL_ERROR(state, XIL_ERROR_CONFIGURATION, "di-5", TRUE);
    }

    XilOpCompress* op = new XilOpCompress(op_number);
    if(op == NULL) {
        XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    // 
    // Only set the src, since the dst is always the cis
    // and this is known via the deviceCompression
    //  
    op->setSrc(1, src_image);
    op->setDst(1, cis);

    //
    //  For deferred compression, the argument to the compression also
    //  includes which frame in the CIS it is compressing into. This
    //  information is used in XilOp::executeDeferred() to create a
    //  connection between a decompress that depends on a compress.
    //
    int write_frame = cis->getWriteFrame();
    op->setParam(1, write_frame);

    int read_frame  = cis->getReadFrame();
    int max_frames  = cis->getMaxFrames();
    int start_frame = cis->getStartFrame();
                             
    if(write_frame >= 0) {
        write_frame++;
        cis->lock();
        cis->setWriteFrame(write_frame);
        cis->unlock();
    }
 
    //
    //  Determine if our perception of start_frame must be adjusted
    //  because max_frames has been set and this compression will put
    //  too many into the buffer.
    //
    if(max_frames>0 && ((write_frame - start_frame) > max_frames)) {

        start_frame = write_frame - max_frames;
        cis->lock();
        cis->setStartFrame(start_frame);
        cis->unlock();
 
        //
        //  If we are going to move start_frame beyond read_frame, we must
        //  send the user an error and adjust read_frame.
        //
        if(read_frame < start_frame) {
            XIL_ERROR(state, XIL_ERROR_USER, "di-115", FALSE);
            cis->lock();
            cis->setReadFrame(start_frame);
            cis->unlock();
        }
 
        //
        //  If start_frame has been adjusted such that there are
        //  outstanding decompressions that are before start_frame,
        //  they must be flushed.  So, we'll run through any dependents
        //
        cis->flushPriorDecompressOps(start_frame);
 
        dc->adjustStart(start_frame);
    }


    return op;
}

XilOpCompress::XilOpCompress(XilOpNumber op_num) : XilOpPoint(op_num) { }
XilOpCompress::~XilOpCompress() { }
