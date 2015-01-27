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
//  File:       Arith.cc
//  Project:    XIL
//  Revision:   1.8
//  Last Mod:   10:13:39, 03/10/00
//
//  Description:
//
//    Implementations of the Arithmetic and Logical functions
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Arith.cc	1.8\t00/03/10  "

#include "XilDeviceManagerComputeMMXBYTE.hh"
#include "ComputeInfoMMX.hh"

XilStatus
XilDeviceManagerComputeMMXBYTE::Add(XilOp*       op,
                                    unsigned     op_count,
                                    XilRoi*      roi,
                                    XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplAdd(&ci.mmxSrc1, &ci.mmxSrc2, &ci.mmxDst, NULL);

            if(IPL_ERRCHK("Add", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}

XilStatus
XilDeviceManagerComputeMMXBYTE::AddConst(XilOp*       op,
                                         unsigned     op_count,
                                         XilRoi*      roi,
                                         XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    Xil_signed16*   consts;
    op->getParam(1, (void**)&consts);

    //
    // Verify that all constants are the same.
    // Otherwise, let memory handle it
    //
    int first_const = (int) consts[0];;
    for(unsigned int i=1; i<ci.destNumBands; i++) {
        if(consts[i] != first_const) {
            return XIL_FAILURE;
        }
    }
    
    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplAddS(&ci.mmxSrc1, &ci.mmxDst, first_const, NULL);

            if(IPL_ERRCHK("AddConst", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}


XilStatus
XilDeviceManagerComputeMMXBYTE::Subtract(XilOp*       op,
                                         unsigned     op_count,
                                         XilRoi*      roi,
                                         XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplSubtract(&ci.mmxSrc1, &ci.mmxSrc2, &ci.mmxDst, FALSE, NULL);

            if(IPL_ERRCHK("Subtract", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}

XilStatus
XilDeviceManagerComputeMMXBYTE::SubtractConst(XilOp*       op,
                                              unsigned     op_count,
                                              XilRoi*      roi,
                                              XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    Xil_signed16*   consts;
    op->getParam(1, (void**)&consts);

    //
    // Verify that all constants are the same.
    // Otherwise, let memory handle it
    //
    int first_const = (int) consts[0];;
    for(unsigned int i=1; i<ci.destNumBands; i++) {
        if(consts[i] != first_const) {
            return XIL_FAILURE;
        }
    }
    

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplSubtractS(&ci.mmxSrc1, &ci.mmxDst, first_const, FALSE, NULL);

            if(IPL_ERRCHK("SubtractConst", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}

XilStatus
XilDeviceManagerComputeMMXBYTE::SubtractFromConst(XilOp*       op,
                                                  unsigned     op_count,
                                                  XilRoi*      roi,
                                                  XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    Xil_signed16*   consts;
    op->getParam(1, (void**)&consts);

    //
    // Verify that all constants are the same.
    // Otherwise, let memory handle it
    //
    int first_const = (int) consts[0];;
    for(unsigned int i=1; i<ci.destNumBands; i++) {
        if(consts[i] != first_const) {
            return XIL_FAILURE;
        }
    }
    
    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplSubtractS(&ci.mmxSrc1, &ci.mmxDst, first_const, TRUE, NULL);

            if(IPL_ERRCHK("SubtractFromConst", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}


XilStatus
XilDeviceManagerComputeMMXBYTE::Multiply(XilOp*       op,
                                         unsigned     op_count,
                                         XilRoi*      roi,
                                         XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplMultiply(&ci.mmxSrc1, &ci.mmxSrc2, &ci.mmxDst, NULL);

            if(IPL_ERRCHK("Multiply", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}


XilStatus
XilDeviceManagerComputeMMXBYTE::MultiplyConst(XilOp*       op,
                                              unsigned     op_count,
                                              XilRoi*      roi,
                                              XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    float*   consts;
    op->getParam(1, (void**)&consts);

    //
    // Verify that all constants are the same,
    // and that they are all non-negative integers.
    // Otherwise, let memory handle it
    //
    float float_const = consts[0];;
    if(float_const != (float)((int)float_const)) {
        return XIL_FAILURE;
    }
    for(unsigned int i=1; i<ci.destNumBands; i++) {
        if(consts[i] != float_const ||
           consts[i] < 0.0F         ||
           consts[i] != (float)((int)consts[i])) {
            return XIL_FAILURE;
        }
    }
    unsigned int first_const = (unsigned int) float_const;
    

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplSubtractS(&ci.mmxSrc1, &ci.mmxDst, first_const, FALSE, NULL);

            if(IPL_ERRCHK("MultiplyConst", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}


XilStatus
XilDeviceManagerComputeMMXBYTE::And(XilOp*       op,
                                    unsigned     op_count,
                                    XilRoi*      roi,
                                    XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplAnd(&ci.mmxSrc1, &ci.mmxSrc2, &ci.mmxDst, NULL);

            if(IPL_ERRCHK("And", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}

XilStatus
XilDeviceManagerComputeMMXBYTE::AndConst(XilOp*       op,
                                         unsigned     op_count,
                                         XilRoi*      roi,
                                         XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    Xil_unsigned8*   consts;
    op->getParam(1, (void**)&consts);

    //
    // Verify that all constants are the same.
    // Otherwise, let memory handle it
    //
    unsigned int first_const = (unsigned int) consts[0];;
    for(unsigned int i=1; i<ci.destNumBands; i++) {
        if(consts[i] != first_const) {
            return XIL_FAILURE;
        }
    }
    
    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplAndS(&ci.mmxSrc1, &ci.mmxDst, first_const, NULL);

            if(IPL_ERRCHK("AndConst", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}


XilStatus
XilDeviceManagerComputeMMXBYTE::Or(XilOp*       op,
                                   unsigned     op_count,
                                   XilRoi*      roi,
                                   XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplOr(&ci.mmxSrc1, &ci.mmxSrc2, &ci.mmxDst, NULL);

            if(IPL_ERRCHK("Or", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}

XilStatus
XilDeviceManagerComputeMMXBYTE::OrConst(XilOp*       op,
                                        unsigned     op_count,
                                        XilRoi*      roi,
                                        XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    Xil_unsigned8*   consts;
    op->getParam(1, (void**)&consts);

    //
    // Verify that all constants are the same.
    // Otherwise, let memory handle it
    //
    unsigned int first_const = (unsigned int) consts[0];;
    for(unsigned int i=1; i<ci.destNumBands; i++) {
        if(consts[i] != first_const) {
            return XIL_FAILURE;
        }
    }
    
    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplAndS(&ci.mmxSrc1, &ci.mmxDst, first_const, NULL);

            if(IPL_ERRCHK("OrConst", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}


XilStatus
XilDeviceManagerComputeMMXBYTE::Xor(XilOp*       op,
                                    unsigned     op_count,
                                    XilRoi*      roi,
                                    XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplXor(&ci.mmxSrc1, &ci.mmxSrc2, &ci.mmxDst, NULL);

            if(IPL_ERRCHK("Xor", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}

XilStatus
XilDeviceManagerComputeMMXBYTE::XorConst(XilOp*       op,
                                         unsigned     op_count,
                                         XilRoi*      roi,
                                         XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    Xil_unsigned8*   consts;
    op->getParam(1, (void**)&consts);

    //
    // Verify that all constants are the same.
    // Otherwise, let memory handle it
    //
    unsigned int first_const = (unsigned int) consts[0];;
    for(unsigned int i=1; i<ci.destNumBands; i++) {
        if(consts[i] != first_const) {
            return XIL_FAILURE;
        }
    }
    
    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplAndS(&ci.mmxSrc1, &ci.mmxDst, first_const, NULL);

            if(IPL_ERRCHK("XorConst", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}


XilStatus
XilDeviceManagerComputeMMXBYTE::Not(XilOp*       op,
                                    unsigned     op_count,
                                    XilRoi*      roi,
                                    XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            if(ci.createIplImages() == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            iplNot(&ci.mmxSrc1, &ci.mmxDst, NULL);

            if(IPL_ERRCHK("Not", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}
