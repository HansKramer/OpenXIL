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
//  File:	XilOpConvolve.cc
//  Project:	XIL
//  Revision:	1.18
//  Last Mod:	10:07:45, 03/10/00
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
#pragma ident	"@(#)XilOpConvolve.cc	1.18\t00/03/10  "

#include <stdlib.h>
#include <string.h>

#include <xil/xilGPI.hh>
#include "XilOpAreaKernel.hh"
#include "XilOpSetValue.hh"
#include "XiliOpUtils.hh"

class XilOpConvolve : public XilOpAreaKernel {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);

    XilStatus     gsForwardMap(XiliRect*     src_rect,
                               unsigned int  src_number,
                               XiliRect*     dst_rect);

    XilStatus     switchToAlternateOp();

protected:
                  XilOpConvolve(XilOpNumber op_num);
    virtual       ~XilOpConvolve();

private:
    Xil_boolean         isSeparableConvolution;
    XilEdgeCondition    edgeCondition;

    static XilOpCache	separableConvolveOpCache;
    static XilOpCache	convolveOpCache;
};

XilStatus
XilOpConvolve::gsForwardMap(XiliRect*     src_rect,
                            unsigned int  src_number,
                            XiliRect*     dst_rect)
{
    if(edgeCondition == XIL_EDGE_NO_WRITE) {
        return XilOpAreaKernel::gsForwardMap(src_rect, src_number, dst_rect);
    } else {
        return XilOp::gsForwardMap(src_rect, src_number, dst_rect);
    }
}

XilOpCache   XilOpConvolve::separableConvolveOpCache;
XilOpCache   XilOpConvolve::convolveOpCache;

//
// Create the convolve op class
//
XilOp*
XilOpConvolve::create(char  function_name[],
                      void* args[],
                      int   )
{
    XilImage*  src    = (XilImage*)args[0];
    XilImage*  dst    = (XilImage*)args[1];
    XilKernel* kernel = (XilKernel*)args[2];

    //
    //  Check the kernel parameter for NULL
    //
    if(kernel == NULL) {
	XIL_ERROR(dst->getSystemState(),XIL_ERROR_USER,"di-221",TRUE);
	return NULL;
    }


    // 
    //  Before creating any op, we need to check for the degenerate
    //  case of a src image that is too small in either X or Y or both
    //  to provide ANY valid source pixels for the destination.
    //
    XilEdgeCondition edge_condition = ((XilEdgeCondition)((int)args[3]));
    Xil_boolean small_source_flag = FALSE;

    if((src->getWidth() < kernel->getWidth()) ||
       (src->getHeight() < kernel->getHeight())) {
        //
        //  What we do in this case depends on the edge condition.
        //  For no_write we want to drop the operation entirely.
        //      since we can't do that, we'll set a flag and generate
        //      an empty intersected roi in generateIntersectedRoi
        //  For zero_fill we'll turn the operation into a set_value op
        //  For edge_extend we'll create a convolve op and mark it later
        //      as a special type of box.
        //
        switch(edge_condition) {
          case XIL_EDGE_ZERO_FILL:
          {
              float* zero_values = new float[dst->getNumBands()];
              for(unsigned int i = dst->getNumBands(); i > 0; i--) {
                  zero_values[i] = 0.0;
              }
              void * args_set_value[3] = {(void*)dst, (void*)zero_values, NULL};
              
              XilOp* new_op = XilOpSetValue::create("set_value", args_set_value, 2);
              
              delete [] zero_values;
              return new_op;
              break;
          }
          case XIL_EDGE_NO_WRITE:
          case XIL_EDGE_EXTEND:
          default:
            //
            //  These cases should go ahead and create the convolve op.
            //  Setting the small_source flag will cause the right
            //  behavior later on.
            //
            small_source_flag = TRUE;
            break;
        }
    }

    //
    //  Is it a separable kernel?
    //
    XilOpNumber	sep_opnum = -1;
    XilOpNumber	opnum     = -1;
    if(kernel->isSeparable()) {
        sep_opnum = xili_verify_op_args("separable_convolve",
                                        &separableConvolveOpCache,
                                        dst, src, NULL, NULL, FALSE);
    }

    //
    //  If it wasn't a separable kernel or the args verification failed, we'll
    //  try to create just a convolve operation -- not a separable operation.
    //
    XilOpConvolve* op;
    if(sep_opnum == -1) {
        if((opnum = xili_verify_op_args(function_name, &convolveOpCache,
                                        dst, src, NULL, NULL, FALSE)) == -1) {
            return NULL;
        }

        op = new XilOpConvolve(opnum);
    } else {
        op = new XilOpConvolve(sep_opnum);

        //
        //  Indicate we've created a separable convolution operation.
        //
        op->isSeparableConvolution = TRUE;
    }

    if(op == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    //
    //  Copy the Kernel
    //
    kernel = (XilKernel*)kernel->createCopy();
    if(kernel == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-163", TRUE);
	return NULL;
    }

    //
    // Kernel is inverted before being send to the compute routine.
    // Yes, it sounds silly, but that is how the operation is defined.
    // 
    kernel->invert();
    
    op->setSrc(1, (XilImage*)args[0]);
    op->setDst(1, dst);
    op->setParam(1, kernel);

    //
    //  Stash the edge condition away in our op for future use.
    //
    op->edgeCondition = ((XilEdgeCondition)((int)args[3]));
    op->setParam(2, op->edgeCondition);

    //
    // Set the parameters for later use, must be done after src image
    // is set up.
    //
    op->initializeAreaKernel(kernel->getWidth(), kernel->getHeight(),
			     kernel->getKeyX(), kernel->getKeyY(),
                             ((XilEdgeCondition)((int)args[3])),
                             small_source_flag);

    return op;
}

XilStatus
XilOpConvolve::switchToAlternateOp()
{
    if(isSeparableConvolution) {
        //
        //  We did create a separable operation.  So, we'll try to switch
        //  ourself into a regular convolve operation.  First, we ensure the
        //  src image is valid -- it may have been set invalid thinking we
        //  couldn't switch or for some other reason.
        //
        getSrcImage(1)->setValid(TRUE);

        XilOpNumber opnum = xili_verify_op_args("convolve", &convolveOpCache,
                                                getDstImage(1),
                                                getSrcImage(1));
        if(opnum == -1) {
            //
            //  We didn't succeed for some reason...
            //
            return XIL_FAILURE;
        }

        setOpNumber(opnum);

        isSeparableConvolution = FALSE;

        return XIL_SUCCESS;
    } else {
        //
        //  We were already a convolve operation.
        //
        return XIL_FAILURE;
    }
}

XilOpConvolve::XilOpConvolve(XilOpNumber op_num) :
    XilOpAreaKernel(op_num)
{
    isSeparableConvolution = FALSE;
}

XilOpConvolve::~XilOpConvolve() { }
