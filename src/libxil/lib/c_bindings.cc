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
//  File:	c_bindings.cc
//  Project:	XIL
//  Revision:	1.140
//  Last Mod:	10:08:03, 03/10/00
//
//  Description:
//	
//	C++ implementation of C-callable bindings for XIL
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)c_bindings.cc	1.140\t00/03/10  "

//
//  C Includes
//
#include "_XilDefines.h"

//
//  XIL Includes
//
#include "_XilGlobalState.hh"
#include "_XilSystemState.hh"
#include "_XilOp.hh"


#include "_XilCis.hh"
#include "_XilColorspace.hh"
#include "_XilColorspaceList.hh"
#include "_XilDitherMask.hh"
#include "_XilImage.hh"
#include "_XilInterpolationTable.hh"
#include "_XilKernel.hh"
#include "_XilLookupSingle.hh"
#include "_XilLookupCombined.hh"
#include "_XilLookupColorcube.hh"
#include "_XilHistogram.hh"
#include "_XilDevice.hh"
#include "_XilRoi.hh"
#include "_XilSel.hh"

#include "XilStorageAPI.hh"


/*
 *  Memory structure for compatibility between C and C++'s understanding
 *  of how memory storage is returned
 */
typedef struct __XilMemoryStorageBit_C {
   Xil_unsigned8* data;            /* pointer to the first byte of the image */
   unsigned int   scanline_stride; /* the number of bytes between scanlines */
   unsigned long  band_stride;     /* the number of bytes between bands */
   unsigned char  offset;          /* the number of bits to the first pixel */
} XilMemoryStorageBit_C;

typedef struct __XilMemoryStorageByte_C {
   Xil_unsigned8* data;            /* pointer to the first byte of the image */
   unsigned long  scanline_stride; /* the number of bytes between scanlines */
   unsigned int   pixel_stride;    /* the number of bytes between pixels */
} XilMemoryStorageByte_C;

typedef struct __XilMemoryStorageShort_C {
   Xil_signed16* data;             /* pointer to the first word of the image */
   unsigned long scanline_stride;  /* the number of words between scanlines */
   unsigned int  pixel_stride;     /* the number of words between pixels */
} XilMemoryStorageShort_C;

/*
 *  Previous releases of XIL had an unsupported/undocumented structure here for
 *  describing floating point data.  We ensure binary compatibility with the
 *  supported interface be replacing it with a place-holder for the
 *  undocumented structure.
 */
typedef struct __XilMemoryStoragePlaceHolderForBackwardCompatibleStructure_C {
   void*         a;
   unsigned long b;
   unsigned int  c;
} XilMemoryStoragePlaceHolderForBackwardCompatibleStructure_C;

typedef union __XilMemoryStorage_C {
    XilMemoryStorageBit_C   bit;
    XilMemoryStorageByte_C  byte;
    XilMemoryStorageShort_C shrt;
    XilMemoryStoragePlaceHolderForBackwardCompatibleStructure_C backward_compatible_struct;
} XilMemoryStorage_C;

//
//  Routine for simplifying the creation and execution of operations.
//
typedef enum __OperationNum {
    XIL_ABSOLUTE,
    XIL_ADD,
    XIL_ADD_CONST,
    XIL_AFFINE,
    XIL_AND,
    XIL_AND_CONST,
    XIL_BAND_COMBINE,
    XIL_BLACK_GENERATION,
    XIL_BLEND,
    XIL_CAPTURE,
    XIL_CAST,
    XIL_CHOOSE_COLORMAP,
    XIL_COLOR_CONVERT,
    XIL_COLOR_CORRECT,
    XIL_COMPRESS,
    XIL_CONVOLVE,
    XIL_COPY,
    XIL_COPY_PATTERN,
    XIL_COPY_WITH_PLANEMASK,
    XIL_DECOMPRESS,
    XIL_DILATE,
    XIL_DISPLAY,
    XIL_DIVIDE,
    XIL_DIVIDE_BY_CONST,
    XIL_DIVIDE_INTO_CONST,
    XIL_EDGE_DETECTION,
    XIL_ERROR_DIFFUSION,
    XIL_ERODE,
    XIL_EXTREMA,
    XIL_FILL,
    XIL_HISTOGRAM_OP,
    XIL_LOOKUP_OP,
    XIL_NEAREST_COLOR,
    XIL_MIN,
    XIL_MAX,
    XIL_MULTIPLY,
    XIL_MULTIPLY_CONST,
    XIL_NOT,
    XIL_OR,
    XIL_OR_CONST,
    XIL_ORDERED_DITHER,
    XIL_PAINT,
    XIL_RESCALE,
    XIL_ROTATE,
    XIL_SCALE,
    XIL_SET_VALUE,
    XIL_SOFT_FILL,
    XIL_SQUEEZE_RANGE,
    XIL_SUBSAMPLE_ADAPTIVE,
    XIL_SUBSAMPLE_BINARY_TO_GRAY,
    XIL_SUBTRACT,
    XIL_SUBTRACT_CONST,
    XIL_SUBTRACT_FROM_CONST,
    XIL_TABLEWARP,
    XIL_TABLEWARP_HORIZONTAL,
    XIL_TABLEWARP_VERTICAL,
    XIL_THRESHOLD,
    XIL_TRANSLATE,
    XIL_TRANSPOSE,
    XIL_XOR,
    XIL_XOR_CONST,
    XIL_UNSHARP,
    XIL_UNSHARP_IC
} OperationNum;

static XilOpCreateFunctionPtr op_create_funcs[300];

//--------------------------------------------------------------------------
//
//  inlines
//
//--------------------------------------------------------------------------

//
//  C bindings need get state that can accept NULL without generating a
//  redundant error.
//
inline
XilSystemState*
_XILI_GET_STATE(XilDeferrableObject* def_obj)
{
    return (def_obj == NULL)? NULL : def_obj->getSystemState();
}

static XilStatus
xili_create_and_execute_op(const char*  op_name,
                           OperationNum opnum,
                           void*        args[],
                           int          num_args)
{
    //
    //  NOTE:  As you can see, the setting and testing of op_create_funcs is
    //         not MT-safe in that it is possible for getXilOpCreateFunc to be
    //         called multiple times by different threads.  This is OK because
    //         getXilOpCreateFunc is MT-safe and the storing of the value in
    //         op_create_funcs is allowed to be done multiple times and it
    //         saves the overhead of an extra mutex lock for every time any
    //         XIL operation is called.
    //
    if(op_create_funcs[opnum] == NULL) {
        op_create_funcs[opnum] =
            XilGlobalState::getXilGlobalState()->getXilOpCreateFunc(op_name);
        if(op_create_funcs[opnum] == NULL) {
            return XIL_FAILURE;
        }
    }

    XilOp* op =
            (*op_create_funcs[opnum])(op_name, args, num_args);

    TNF_PROBE_2(xilop_create, "xilop", "xilop_create",
                tnf_opaque, "this", op,
                tnf_string, "opname", op_name);

    if(op == NULL) {
        return XIL_FAILURE;
    } else {
        op->insert();
    }
    
    return XIL_SUCCESS;
}


extern "C" {

XilSystemState*
xil_open()
{
    return XiliOpen();
}

void
xil_close(XilSystemState* system_state)
{
    (XilGlobalState::getXilGlobalState())->destroySystemState(system_state);
}


#include "../include/version.h"


const char *
xil_get_version()
{
    static char version[100];

    sprintf(version, "%d.%d.%d", XIL_API_MAJOR_VERSION, 
                                 XIL_API_MINOR_VERSION,
                                 XIL_API_REVISION);

    return version;
}


#include "../include/_XilDeviceIO.hh"


Drawable
xil_get_drawable(XilImage image)
{
    XilDeviceIO* device_io = image.getDeviceIO();
    if (device_io) {
        Drawable window;
        if (device_io->getAttribute("DRAWABLE", (void **) &window) == XIL_SUCCESS)
            return window;
    }

    return NULL;
}


Display*
xil_get_display(XilImage image)
{
    XilDeviceIO* device_io = image.getDeviceIO();
    if (device_io) {
        Display *display;
        if (device_io->getAttribute("DISPLAY", (void **) &display) == XIL_SUCCESS)
            return display;
    }

    return NULL;
}


XilImage*
xil_create(XilSystemState* system_state,
           unsigned int    width,
           unsigned int    height,
           unsigned int    nbands,
           XilDataType     datatype)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilImage(width, height, nbands, datatype);
}

XilImage*
xil_create_from_type(XilSystemState* system_state,
                     XilImageFormat* image_format)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilImage(image_format);
}

XilImage*
xil_create_temporary(XilSystemState* system_state,
                     unsigned int    width,
                     unsigned int    height,
                     unsigned int    nbands,
                     XilDataType     datatype)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilImageTemp(width, height,
                                            nbands, datatype);
}

XilImage*
xil_create_temporary_from_type(XilSystemState* system_state,
                               XilImageFormat* image_format)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilImageTemp(image_format);
}

XilImage*
xil_create_child(XilImage*    src, 
                 unsigned int xstart,
                 unsigned int ystart, 
                 unsigned int width,
                 unsigned int height, 
                 unsigned int startband,
                 unsigned int numbands)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-207", src);

    src->lock();

    XilImage* child = src->createChild(xstart,
                                       ystart,
                                       width,
                                       height,
                                       startband,
                                       numbands);
    src->unlock();

    return child;
}

XilImage*
xil_create_copy(XilImage* src, 
		unsigned int xstart,
                unsigned int ystart, 
		unsigned int width,
                unsigned int height,
		unsigned int startband,
                unsigned int numbands)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-207", src);

    src->lock();

    XilImage* copy = src->createCopy(xstart,
                                     ystart,
                                     width,
                                     height,
                                     startband,
                                     numbands);

    src->unlock();

    return copy;
}

XilImage*
xil_create_from_device(XilSystemState* state,
                       char*           devicename,
                       XilDevice*      device)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return state->createXilImage(devicename, device);
}

XilImage*
xil_create_from_window(XilSystemState* system_state,
                       Display*        display,
                       Window          window)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilImage(display, window);
}

XilImage*
xil_create_double_buffered_window(XilSystemState* system_state,
                                  Display*        display,
                                  Window          window)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilImage(display, window, TRUE);
}

XilImage*
xil_create_from_special_window(XilSystemState* system_state,
                               Display*        display,
                               Window          window,
                               XilWindowCaps   wincaps)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilImage(display, window, wincaps);
}



void
xil_set_active_buffer(XilImage*   image,
                      XilBufferId id)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->setActiveBuffer(id);

    image->unlock();
}

XilBufferId
xil_get_active_buffer(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_BACK_BUFFER, "di-207", image);

    image->lock();

    XilBufferId id = image->getActiveBuffer();

    image->unlock();

    return id;
}

void
xil_swap_buffers(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->swapBuffers();

    image->unlock();
}

void
xil_sync(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->sync();

    image->unlock();
}

void
xil_toss(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->toss();

    image->unlock();
}

void
xil_destroy(XilImage* image)
{
    if(image == NULL) {
        return;
    }

    image->lock();

    //
    //  Destroying the image will take care of releasing the lock.
    //
    image->destroy();
}

XilObjectType
xil_object_get_type(XilObject* object)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_IMAGE, "di-268", object);

    object->lock();

    XilObjectType type = object->getType();

    object->unlock();

    return type;
}

XilImageFormat*
xil_get_imagetype(XilImage* image)
{
    return (XilImageFormat*)image;
}

//
//  ALL XIL Operations
//
//  The calls in xili_create_and_execute_op() are MT-SAFE.
//
void
xil_copy(XilImage* src,
         XilImage* dst)
{
    void* args[3] = { (void*)src, (void*)dst, NULL };

    if(xili_create_and_execute_op("copy", XIL_COPY, args, 2) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_add(XilImage* src1,
        XilImage* src2,
        XilImage* dst)
{
    void* args[4] = { (void*)src1, (void*)src2, (void*)dst, NULL };

    if(xili_create_and_execute_op("add", XIL_ADD, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_subtract(XilImage* src1,
             XilImage* src2,
             XilImage* dst)
{
    void* args[4] = { (void*)src1, (void*)src2, (void*)dst, NULL };

    if(xili_create_and_execute_op("subtract", XIL_SUBTRACT, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_multiply(XilImage* src1,
             XilImage* src2,
             XilImage* dst)
{
    void* args[4] = { (void*)src1, (void*)src2, (void*)dst, NULL };

    if(xili_create_and_execute_op("multiply", XIL_MULTIPLY, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_divide(XilImage* src1,
           XilImage* src2,
           XilImage* dst)
{
    void* args[4] = { (void*)src1, (void*)src2, (void*)dst, NULL };

    if(xili_create_and_execute_op("divide", XIL_DIVIDE, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_min(XilImage* src1,
        XilImage* src2,
        XilImage* dst)
{
    void* args[4] = { (void*)src1, (void*)src2, (void*)dst, NULL };

    if(xili_create_and_execute_op("min", XIL_MIN, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_max(XilImage* src1,
        XilImage* src2,
        XilImage* dst)
{
    void* args[4] = { (void*)src1, (void*)src2, (void*)dst, NULL };

    if(xili_create_and_execute_op("max", XIL_MAX, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_and(XilImage* src1,
        XilImage* src2,
        XilImage* dst)
{
    void* args[4] = { (void*)src1, (void*)src2, (void*)dst, NULL };

    if(xili_create_and_execute_op("and", XIL_AND, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_or(XilImage* src1,
       XilImage* src2,
       XilImage* dst)
{
    void* args[4] = { (void*)src1, (void*)src2, (void*)dst, NULL };

    if(xili_create_and_execute_op("or", XIL_OR, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_xor(XilImage* src1,
        XilImage* src2,
        XilImage* dst)
{
    void* args[4] = { (void*)src1, (void*)src2, (void*)dst, NULL };

    if(xili_create_and_execute_op("xor", XIL_XOR, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_add_const(XilImage* src,
              float*    vector,
              XilImage* dst)
{
    void* args[4] = { (void*)src, (void*)vector, (void*)dst, NULL };

    if(xili_create_and_execute_op("add_const", XIL_ADD_CONST, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_subtract_const(XilImage* src,
                   float*    vector,
                   XilImage* dst)
{
    void* args[4] = { (void*)src, (void*)vector, (void*)dst, NULL };

    if(xili_create_and_execute_op("subtract_const",
                                  XIL_SUBTRACT_CONST, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_subtract_from_const(float*    vector,
                        XilImage* src,
                        XilImage* dst)
{
    void* args[4] = { (void*)vector, (void*)src, (void*)dst, NULL };

    if(xili_create_and_execute_op("subtract_from_const",
                                  XIL_SUBTRACT_FROM_CONST, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_multiply_const(XilImage* src,
                   float*    vector,
                   XilImage* dst)
{
    void* args[4] = { (void*)src, (void*)vector, (void*)dst, NULL };

    if(xili_create_and_execute_op("multiply_const",
                                  XIL_MULTIPLY_CONST, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_divide_by_const(XilImage* src,
                    float*    vector,
                    XilImage* dst)
{
    void* args[4] = { (void*)src, (void*)vector, (void*)dst, NULL };

    if(xili_create_and_execute_op("divide_by_const",
                                  XIL_DIVIDE_BY_CONST, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_divide_into_const(float*    vector,
                      XilImage* src,
                      XilImage* dst)
{
    void* args[4] = { (void*)vector, (void*)src, (void*)dst, NULL };

    if(xili_create_and_execute_op("divide_into_const",
                                  XIL_DIVIDE_INTO_CONST, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_and_const(XilImage*     src,
              unsigned int* vector,
              XilImage*     dst)
{
    void* args[4] = { (void*)src, (void*)vector, (void*)dst, NULL };

    if(xili_create_and_execute_op("and_const", XIL_AND_CONST, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_or_const(XilImage*     src,
             unsigned int* vector,
             XilImage*     dst)
{
    void* args[4] = { (void*)src, (void*)vector, (void*)dst, NULL };

    if(xili_create_and_execute_op("or_const", XIL_OR_CONST, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_xor_const(XilImage*     src,
              unsigned int* vector,
              XilImage*     dst)
{
    void* args[4] = { (void*)src, (void*)vector, (void*)dst, NULL};

    if(xili_create_and_execute_op("xor_const", XIL_XOR_CONST, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_not(XilImage* src,
        XilImage* dst)
{
    void* args[3] = { (void*)src, (void*)dst, NULL };

    if(xili_create_and_execute_op("not", XIL_NOT, args, 2) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_absolute(XilImage* src,
             XilImage* dst)
{
    void* args[3] = { (void*)src, (void*)dst, NULL };

    if(xili_create_and_execute_op("absolute", XIL_ABSOLUTE, args, 2) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_black_generation(XilImage* src,
                     XilImage* dst,
                     float     black,
                     float     undercolor)
{
    void* args[5] = { (void*)src, (void*)dst, (void*)&black, (void*)&undercolor, NULL };

    if(xili_create_and_execute_op("black_generation",
                                  XIL_BLACK_GENERATION, args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_blend(XilImage* src1,
          XilImage* src2,
          XilImage* dst,
          XilImage* alpha)
{
    void* args[5] = { (void*)src1, (void*)src2, (void*)dst, (void*)alpha, NULL };

    if(xili_create_and_execute_op("blend", XIL_BLEND, args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_cast(XilImage* src,
         XilImage* dst)
{
    void* args[3] = { (void*)src, (void*)dst, NULL };

    if(xili_create_and_execute_op("cast", XIL_CAST, args, 2) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_copy_with_planemask(XilImage*     src,
                        XilImage*     dst,
                        unsigned int* vector)
{
    void* args[4] = { (void*)src, (void*)dst, (void*)vector, NULL };

    if(xili_create_and_execute_op("copy_with_planemask",
                                  XIL_COPY_WITH_PLANEMASK, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_rescale(XilImage* src,
            XilImage* dst,
            float*    scale,
            float*    offset)
{
    void* args[5] = { (void*)src, (void*)dst, (void*)scale, (void*)offset, NULL };

    if(xili_create_and_execute_op("rescale", XIL_RESCALE, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_set_value(XilImage* dst,
              float*    values)
{
    void* args[3] = { (void*)dst, (void*)values, NULL };

    if(xili_create_and_execute_op("set_value", XIL_SET_VALUE, args, 2) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_threshold(XilImage* src,
              XilImage* dst,
              float*    lowvalue,
              float*    highvalue,
              float*    mapvalue)
{
    void* args[6] = { (void*)src, (void*)dst, (void*)lowvalue, (void*)highvalue, (void*)mapvalue, NULL };

    if(xili_create_and_execute_op("threshold", XIL_THRESHOLD, args, 5) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_nearest_color(XilImage*  src,
                  XilImage*  dst,
                  XilLookup* cmap)
{
    void* args[4] = { (void*)src, (void*)dst, (void*)cmap, NULL };

    if(xili_create_and_execute_op("nearest_color",
                                  XIL_NEAREST_COLOR, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_error_diffusion(XilImage*  src,
                    XilImage*  dst,
		    XilLookup* cmap, 
		    XilKernel* distribution)
{
    void* args[5] = {(void*)src, (void*)dst, (void*)cmap, (void*)distribution, NULL};

    if(xili_create_and_execute_op("error_diffusion",
                                  XIL_ERROR_DIFFUSION, args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_extrema(XilImage* src,
            float*    max,
            float*    min)
{
    void* args[4] = { (void*)src, (void*)max, (void*)min, NULL };

    if(xili_create_and_execute_op("extrema", XIL_EXTREMA, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(src), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

XilLookup*
xil_squeeze_range(XilImage* src)
{
    XilLookup*	lut;

    void* args[3] = { (void*)src, (void*)&lut, NULL };

    if(xili_create_and_execute_op("squeeze_range",
                                  XIL_SQUEEZE_RANGE, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(src), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }

    return lut;
}

void 
xil_tablewarp(XilImage* src,
              XilImage* dst,
              char*     interpolation,
	      XilImage* warp_table)
{
    void* args[5] = { (void*)src, (void*)dst, (void*)interpolation, (void*)warp_table, NULL };

    if(xili_create_and_execute_op("tablewarp", XIL_TABLEWARP, args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_tablewarp_horizontal(XilImage* src,
                         XilImage* dst,
                         char*     interpolation,
			 XilImage* warp_table)
{
    void* args[5] = { (void*)src, (void*)dst, (void*)interpolation, (void*)warp_table, NULL };

    if(xili_create_and_execute_op("tablewarp_horizontal",
                                  XIL_TABLEWARP_HORIZONTAL, args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_tablewarp_vertical(XilImage* src,
                       XilImage* dst,
                       char*     interpolation,
		       XilImage* warp_table)
{
    void* args[5] = { (void*)src, (void*)dst, (void*)interpolation, (void*)warp_table, NULL };

    if(xili_create_and_execute_op("tablewarp_vertical",
                                  XIL_TABLEWARP_VERTICAL, args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void 
xil_scale(XilImage* src,
          XilImage* dst,
          char*     interpolation,
	  float     xscale,
          float     yscale)
{
    void* args[6] = { (void*)src, (void*)dst, (void*)interpolation, (void*)&xscale, (void*)&yscale, NULL };

    if(xili_create_and_execute_op("scale", XIL_SCALE, args, 5) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_subsample_adaptive(XilImage* src,
                       XilImage* dst,
                       float     xscale,
                       float     yscale)
{
    void* args[5] = { (void*)src, (void*)dst, (void*)&xscale, (void*)&yscale, NULL };

    if(xili_create_and_execute_op("subsample_adaptive",
                                  XIL_SUBSAMPLE_ADAPTIVE, args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void 
xil_subsample_binary_to_gray(XilImage* src,
                             XilImage* dst,
                             float     xscale, 
			     float     yscale)
{
    void* args[5] = { (void*)src, (void*)dst, (void*)&xscale, (void*)&yscale, NULL };

    if(xili_create_and_execute_op("subsample_binary_to_gray",
                                  XIL_SUBSAMPLE_BINARY_TO_GRAY,
                                  args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_rotate(XilImage* src,
           XilImage* dst,
           char*     interpolation,
           float     angle)
{
    void* args[5] = { (void*)src, (void*)dst, (void*)interpolation, (void*)&angle, NULL };

    if(xili_create_and_execute_op("rotate", XIL_ROTATE, args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_translate(XilImage* src,
              XilImage* dst,
              char*     interpolation,
	      float     xoffset,
              float     yoffset)
{
    void* args[6] = { (void*)src, (void*)dst, (void*)interpolation, (void*)&xoffset, (void*)&yoffset, NULL };

    if(xili_create_and_execute_op("translate", XIL_TRANSLATE, args, 5) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_affine(XilImage* src,
           XilImage* dst,
           char*     interpolation,
	   float*    matrix)
{
    void* args[5] = { (void*)src, (void*)dst, (void*)interpolation, (void*)matrix, NULL };

    if(xili_create_and_execute_op("affine", XIL_AFFINE, args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void 
xil_transpose(XilImage*   src,
              XilImage*   dst,
              XilFlipType fliptype)
{
    void* args[4] = { (void*)src, (void*)dst, (void*)fliptype, NULL };

    if(xili_create_and_execute_op("transpose", XIL_TRANSPOSE, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_band_combine(XilImage*  src,
                 XilImage*  dst,
                 XilKernel* kernel)
{
    void* args[4] = { (void*)src, (void*)dst, (void*)kernel, NULL };

    if(xili_create_and_execute_op("band_combine",
                                  XIL_BAND_COMBINE, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_lookup(XilImage*  src,
           XilImage*  dst,
           XilLookup* lookup)
{
    void* args[4] = { (void*)src, (void*)dst, (void*)lookup, NULL };

    if(xili_create_and_execute_op("lookup", XIL_LOOKUP_OP, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_histogram(XilImage*     src,
              XilHistogram* histogram,
	      unsigned int  skip_x,
              unsigned int  skip_y)
{
    void* args[5] = { (void*)src, (void*)histogram, (void*)skip_x, (void*)skip_y, NULL };

    if(xili_create_and_execute_op("histogram",
                                  XIL_HISTOGRAM_OP, args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(src), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_copy_pattern(XilImage* src,
                 XilImage* dst)
{
    void* args[3] = {(void*)src, (void*)dst, NULL};

    if(xili_create_and_execute_op("copy_pattern",
                                  XIL_COPY_PATTERN, args, 2) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_erode(XilImage* src,
          XilImage* dst,
          XilSel*   sel)
{
    void* args[4] = {(void*)src, (void*)dst, (void*)sel, NULL};

    if(xili_create_and_execute_op("erode", XIL_ERODE, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_dilate(XilImage* src,
           XilImage* dst,
           XilSel*   sel)
{
    void* args[4] = {(void*)src, (void*)dst, (void*)sel, NULL};

    if(xili_create_and_execute_op("dilate", XIL_DILATE, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_convolve(XilImage*        src,
	     XilImage*        dst,
	     XilKernel*       kernel,
	     XilEdgeCondition edge_condition)
{
    void* args[5] = {(void*)src, (void*)dst, (void*)kernel, (void*)edge_condition, NULL};

    if(xili_create_and_execute_op("convolve", XIL_CONVOLVE, args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_unsharp_set_mode(XilSystemState* system_state, XilUnsharpMasking mode)
{
    system_state->set_unsharp_mode(mode);
}
 
 
XilUnsharpMasking
xil_unsharp_get_mode(XilSystemState* system_state)
{
    return system_state->get_unsharp_mode();
}
 
 
void
xil_unsharp(XilImage*    src,
            XilImage*    dst,
            unsigned int size,
            float        alpha,
            float        beta,
            float        gamma)
{
    void* args[7] = {(void*)src, (void*)dst, (void *) &size, (void*) &alpha, (void*) &beta, (void*) &gamma, NULL};
 
    if (xili_create_and_execute_op("unsharp", XIL_UNSHARP, args, 6) == XIL_FAILURE)
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
}
 

void
xil_unsharp_ic(XilImage*             src,
               XilImage*             dst,
               unsigned int          size,
               float                 alpha,
               float                 beta,
               float                 gamma,
               XilUnsharpMaskingType type,
               int                   window,
               int                   level)
{
    void* args[10] = {(void*) src,   (void*) dst,     (void *) &size, (void*) &alpha, (void*) &beta, (void*) &gamma, 
                      (void*) &type, (void*) &window, (void*) &level, NULL};
 
    if (xili_create_and_execute_op("unsharp_ic", XIL_UNSHARP_IC, args, 9) == XIL_FAILURE)
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
}
 

void
xil_edge_detection(XilImage* src,
		   XilImage* dst,
		   XilEdgeDetection method)
{
    void* args[4] = {(void*)src, (void*)dst, (void*)method, NULL};

    if(xili_create_and_execute_op("edge_detection",
                                  XIL_EDGE_DETECTION, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_ordered_dither(XilImage*      src,
                   XilImage*      dst,
                   XilLookup*     cmap,
                   XilDitherMask* mask)
{
    void* args[5] = {(void*)src, (void*)dst, (void*)cmap, (void*)mask, NULL};

    if(xili_create_and_execute_op("ordered_dither",
                                  XIL_ORDERED_DITHER, args, 4) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void 
xil_paint(XilImage*    src,
	  XilImage*    dst,
	  float*       color,
	  XilKernel*   brush,
	  unsigned int count,
	  float*       coord_list)
{
    void* args[7] = {(void*)src, (void*)dst, (void*)color, (void*)brush, (void*)count, (void*)coord_list, NULL};

    if(xili_create_and_execute_op("paint",
				  XIL_PAINT, args, 6) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_fill(XilImage* src,
	 XilImage* dst,
	 float     xseed,
	 float     yseed,
	 float*    boundary,
	 float*    fill_color)
{
    void* args[7] = {(void*)src, (void*)dst, (void*)&xseed, (void*)&yseed, (void*)boundary, (void*)fill_color, NULL};

    if(xili_create_and_execute_op("fill", XIL_FILL, args, 6) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_soft_fill(XilImage*    src,
	      XilImage*    dst,
	      float        xseed,
	      float        yseed,
	      float*       fgcolor,
	      unsigned int num_bgcolor,
	      float*       bgcolor,
	      float*       fill_color)
{
    void* args[9] = { (void*)src, (void*)dst, (void*)&xseed, (void*)&yseed, (void*)fgcolor,
                      (void*)num_bgcolor, (void*)bgcolor, (void*)fill_color, NULL};
    
    if(xili_create_and_execute_op("soft_fill",
                                  XIL_SOFT_FILL, args, 8) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_color_convert(XilImage* src,
		  XilImage* dst)
{
    void* args[3] = {(void*)src, (void*)dst, NULL};

    if(xili_create_and_execute_op("color_convert",
                                  XIL_COLOR_CONVERT, args, 2) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
}

void
xil_color_correct(XilImage*          src,
		  XilImage*          dst,
                  XilColorspaceList* colorspacelist)
{

#ifdef SOLARIS
#ifndef GCC
    void* args[4] = {(void*)src, (void*)dst, (void*)colorspacelist, NULL};

    if(xili_create_and_execute_op("color_correct",
                                  XIL_COLOR_CORRECT, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }
#else
    XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-5", FALSE);
#endif
#else
    //
    // This routine depends on KCMS which is supported only on Solaris.
    //
    XIL_ERROR(_XILI_GET_STATE(dst), XIL_ERROR_SYSTEM, "di-5", FALSE);
#endif
}

XilLookup*
xil_choose_colormap(XilImage*    src,
		    unsigned int size)
{
    XilLookup*	lut;

    void* args[4] = { (void*)src, (void*)&size, (void*)&lut, NULL };
    if(xili_create_and_execute_op("choose_colormap",
                          XIL_CHOOSE_COLORMAP, args, 3) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(src), XIL_ERROR_SYSTEM, "di-21", FALSE);
    }

    return lut;
}

void
xil_compress(XilImage* src,
             XilCis*   cis)
{
    void* args[3] = { (void*)src, (void*)cis, NULL };

    if(xili_create_and_execute_op("compress",
                                  XIL_COMPRESS, args, 2) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(cis),XIL_ERROR_SYSTEM,"di-21",FALSE);
    }
}

void
xil_decompress(XilCis*   cis,
               XilImage* dst)
{
    void* args[3] = { (void*)cis, (void*)dst, NULL };

    if(xili_create_and_execute_op("decompress",
                                  XIL_DECOMPRESS, args, 2) == XIL_FAILURE) {
        XIL_ERROR(_XILI_GET_STATE(cis),XIL_ERROR_SYSTEM,"di-21",FALSE);
    }
}


//------------------------------------------------------------------------
//
//  Begin XIL Object Methods
//	
//------------------------------------------------------------------------

//
//  Methods Common to All XIL Objects
//
//
//  Get the state that was used to create the given object.
//
XilSystemState*
xil_get_state(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-207", image);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return image->getSystemState();
}

XilSystemState*
xil_imagetype_get_state(XilImageFormat* image_format)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-207", image_format);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return image_format->getSystemState();
}

XilSystemState*
xil_lookup_get_state(XilLookup* lookup)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-131", lookup);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return lookup->getSystemState();
}

XilSystemState*
xil_cis_get_state(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-117", cis);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return cis->getSystemState();
}

XilSystemState*
xil_dithermask_get_state(XilDitherMask* dither_mask)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-222", dither_mask);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return dither_mask->getSystemState();
}

XilSystemState*
xil_kernel_get_state(XilKernel* kernel)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-221", kernel);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return kernel->getSystemState();
}

XilSystemState*
xil_sel_get_state(XilSel* sel)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-262", sel);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return sel->getSystemState();
}

XilSystemState*
xil_roi_get_state(XilRoi* roi)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-257", roi);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return roi->getSystemState();
}

XilSystemState*
xil_histogram_get_state(XilHistogram* histogram)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-265", histogram);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return histogram->getSystemState();
}

XilSystemState*
xil_storage_get_state(XilStorageAPI* storage)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-383", storage);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return storage->getSystemState();
}

XilSystemState*
xil_colorspace_get_state(XilColorspace* colorspace)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-296", colorspace);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return colorspace->getSystemState();
}

XilSystemState*
xil_colorspacelist_get_state(XilColorspaceList* colorspacelist)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-372", colorspacelist);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return colorspacelist->getSystemState();
}

//
//  XilSystemState methods -- XilSystemState is MT-SAFE and tests for NULL
//
Xil_boolean
xil_state_get_synchronize(XilSystemState* system_state)
{
    return system_state->getSynchronized();
}
 
void
xil_state_set_synchronize(XilSystemState* system_state,
                          Xil_boolean     onoff)
{
    system_state->setSynchronized(onoff);
}

XilTilingMode
xil_state_get_default_tiling_mode(XilSystemState* system_state)
{
    return system_state->getDefaultTilingMode();
}
 
int
xil_state_set_default_tiling_mode(XilSystemState* system_state,
                                  XilTilingMode   mode)
{
    return system_state->setDefaultTilingMode(mode);
}

int
xil_state_get_default_tilesize(XilSystemState* system_state,
                               unsigned int*   txsize_ptr,
                               unsigned int*   tysize_ptr)
{
    return system_state->getDefaultTileSize(txsize_ptr, tysize_ptr);
}
 
int
xil_state_set_default_tilesize(XilSystemState* system_state,
                               unsigned int    txsize,
                               unsigned int    tysize)
{
    return system_state->setDefaultTileSize(txsize, tysize);
}

void
xil_state_set_interpolation_tables(XilSystemState*        system_state, 
                                   XilInterpolationTable* horiz_kernel, 
                                   XilInterpolationTable* vertical_kernel)
{
    system_state->setInterpolationTables(horiz_kernel, vertical_kernel);
}

void
xil_state_get_interpolation_tables(XilSystemState*         system_state, 
                                   XilInterpolationTable** horiz_kernel, 
                                   XilInterpolationTable** vertical_kernel) 
{
    system_state->getInterpolationTables(horiz_kernel, vertical_kernel);
}

int
xil_state_get_show_action(XilSystemState* system_state)
{
    return system_state->getShowActionFlag();
}

void
xil_state_set_show_action(XilSystemState* system_state,
                          int             on_off)
{
    system_state->setShowActionFlag(on_off);
}

int
xil_state_get_provide_warnings(XilSystemState* system_state)
{
    return system_state->getProvideWarningsFlag();
}

void
xil_state_set_provide_warnings(XilSystemState* system_state,
                               int             on_off)
{
    system_state->setProvideWarningsFlag(on_off);
}



//
//  Lookup Tables
//
XilLookup*     
xil_lookup_create(XilSystemState* system_state,
		  XilDataType     input_datatype,
		  XilDataType     output_datatype,
		  unsigned int    output_nbands, 
		  unsigned int    num_entries,
		  short           first_entry_offset,
                  void*           data)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilLookupSingle(input_datatype, 
                                               output_datatype,
                                               output_nbands,
                                               num_entries,
                                               first_entry_offset,
                                               data);
}

XilLookup*	
xil_lookup_create_combined(XilSystemState* system_state,
			   XilLookup*      lookup_list[],
			   unsigned int    num_lookups)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilLookupCombined((XilLookupSingle**)lookup_list,
                                                 num_lookups);
}

XilLookup*	
xil_colorcube_create(XilSystemState* system_state,
		     XilDataType     input_type,
		     XilDataType     output_type,
		     unsigned int    nbands,
		     short           offset,
		     int*            multipliers,
		     unsigned int*   dimensions)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilLookupColorcube(input_type,
                                                  output_type, 
                                                  nbands,
                                                  offset,
                                                  multipliers,
                                                  dimensions);
}

void	
xil_lookup_destroy(XilLookup* lookup)
{
    if(lookup == NULL) {
        return;
    }

    lookup->lock();

    //
    //  Destroying the lookup will take care of releasing the lock.
    //
    lookup->destroy();
}

XilLookup*	
xil_lookup_get_band_lookup(XilLookup*   lookup, 
			   unsigned int band_num)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-131", lookup);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return (XilLookup*)
           ((XilLookupCombined*)lookup)->getBandLookup(band_num);
}

XilDataType
xil_lookup_get_input_datatype(XilLookup* lookup)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_BIT, "di-131", lookup);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return lookup->getInputDataType();
}

XilDataType	
xil_lookup_get_output_datatype(XilLookup* lookup)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_BIT, "di-131", lookup);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return lookup->getOutputDataType();
}

unsigned int	
xil_lookup_get_output_nbands(XilLookup* lookup)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-131", lookup);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return (unsigned int)lookup->getOutputNBands();
}

unsigned int	
xil_lookup_get_input_nbands(XilLookup* lookup)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-131", lookup);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return (unsigned int)lookup->getInputNBands();
}

unsigned int	
xil_lookup_get_num_entries(XilLookup* lookup)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-131", lookup);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return ((XilLookupSingle*)lookup)->getNumEntries();
}

XilVersionNumber
xil_lookup_get_version(XilLookup* lookup)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-131", lookup);

    XilVersion version;

    lookup->lock();

    lookup->getVersion(&version);

    lookup->unlock();

    //
    //  Here we swap the order of the words o applications compiled
    //  with unsigned long (due to ANSI limitations on absence of long long
    //  use) will see the least significant word.  This means they get
    //  32-bits of uniqueness and they will see the version number
    //  change.
    //
    XilULongLong tmp2;

    *((unsigned long*)&tmp2)       = *(((unsigned long*)&version.versionNumber) + 1);
    *(((unsigned long*)&tmp2) + 1) = *((unsigned long*)&version.versionNumber);

    return tmp2;
}

short
xil_lookup_get_offset(XilLookup* lookup)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-131", lookup);

    lookup->lock();

    short offset = ((XilLookupSingle*)lookup)->getOffset();

    lookup->unlock();

    return offset;
}

void 
xil_lookup_set_offset(XilLookup* lookup,
                      short      offset)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-131", lookup);

    lookup->lock();

    ((XilLookupSingle*)lookup)->setOffset(offset);

    lookup->unlock();
}

Xil_boolean	
xil_lookup_get_colorcube(XilLookup* lookup)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-131", lookup);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return lookup->isColorcube();
}

Xil_boolean	
xil_lookup_get_colorcube_info(XilLookup*    lookup,
			      int*          multipliers,
                              unsigned int* dimensions, 
			      short*        origin)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-131", lookup);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return ((XilLookupColorcube*)lookup)->getColorcubeInfo(multipliers,
                                                           dimensions, 
                                                           origin);
}

void
xil_lookup_get_values(XilLookup*   lookup, 
		      short        start,
                      unsigned int num_values,
                      void*        data)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-131", lookup);

    lookup->lock();

    ((XilLookupSingle*)lookup)->getValues(start, num_values, data);

    lookup->unlock();
}

void
xil_lookup_set_values(XilLookup*   lookup, 
		      short        start,
                      unsigned int num_values,
                      void*        data)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-131", lookup);

    lookup->lock();

    ((XilLookupSingle*)lookup)->setValues(start, num_values, data);

    lookup->unlock();
}

XilLookup*	
xil_lookup_create_copy(XilLookup* lookup)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-131", lookup);

    lookup->lock();

    XilLookup* copy = (XilLookup*)lookup->createCopy();

    lookup->unlock();

    return copy;
}

XilLookup*
xil_lookup_convert(XilLookup* src,
		   XilLookup* dst)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-131", src);
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-131", dst);

    //
    //  TODO: 9/7/96 jlf  Maybe just a static lock?  Need to devise a method
    //                    for locking both lookups. 
    //
    return src->convert(dst);
}

//
// Histograms
//
XilHistogram*
xil_histogram_create(XilSystemState* system_state,
		     unsigned int    nbands, 
		     unsigned int*   nbins,
		     float*          low_value,
		     float*          high_value)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilHistogram(nbands, nbins, low_value,
                                            high_value);
}

XilHistogram*	
xil_histogram_create_copy(XilHistogram* histogram)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-265", histogram);

    histogram->lock();

    XilHistogram* copy = (XilHistogram*)histogram->createCopy();

    histogram->unlock();

    return copy;
}

unsigned int
xil_histogram_get_nbands(XilHistogram* histogram)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-265", histogram);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return histogram->getNumBands();
}

void
xil_histogram_get_nbins(XilHistogram* histogram,
			unsigned int* nbins)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-265", histogram);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    histogram->getNumBins(nbins);
}

void
xil_histogram_get_limits(XilHistogram* histogram,
			 float*        low_value,
                         float*        high_value)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-265", histogram);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    histogram->getLowValues(low_value);

    if(histogram != NULL) {
        histogram->getHighValues(high_value);
    }
}

void
xil_histogram_get_values(XilHistogram*   histogram, 
                         unsigned int*   data)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-131", histogram);

    histogram->lock();

    histogram->getValues(data);

    histogram->unlock();
}

void
xil_histogram_get_info(XilHistogram* histogram,
		       unsigned int* nbands,
		       unsigned int* nbins,
		       float*        low_value,
		       float*        high_value)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-265", histogram);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    if(nbands) {
        *nbands = histogram->getNumBands();
    }

    if(histogram) {
	histogram->getNumBins(nbins);
	histogram->getLowValues(low_value);
	histogram->getHighValues(high_value);
    }
}

void		
xil_histogram_destroy(XilHistogram* histogram)
{
    if(histogram == NULL) {
        return;
    }

    histogram->lock();

    //
    //  Destroying the histogram will take care of releasing the lock.
    //
    histogram->destroy();
}


//
// Kernels
//
XilKernel*
xil_kernel_create(XilSystemState* system_state,
                  unsigned int    width,
                  unsigned int    height, 
                  unsigned int    keyx,
                  unsigned int    keyy,
                  float*          data)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilKernel(width, height, keyx, keyy, data);
}

XilKernel*
xil_kernel_create_separable(XilSystemState* system_state,
                            unsigned int    width,
                            unsigned int    height,
                            unsigned int    keyx,
                            unsigned int    keyy,
                            float*          x_data,
                            float*          y_data)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilKernel(width,  height,
                                         keyx,   keyy,
                                         x_data, y_data);
}

XilKernel*
xil_kernel_create_copy(XilKernel* kernel)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-221", kernel);

    kernel->lock();    

    XilKernel* copy = (XilKernel*)kernel->createCopy();

    kernel->unlock();

    return copy;
}

void
xil_kernel_destroy(XilKernel* kernel)
{
    if(kernel == NULL) {
        return;
    }

    kernel->lock();

    //
    //  Destroying the kernel will take care of releasing the lock.
    //
    kernel->destroy();
}

unsigned int
xil_kernel_get_width(XilKernel* kernel)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-221", kernel);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return (unsigned int)kernel->getWidth();
}

unsigned int
xil_kernel_get_height(XilKernel* kernel)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-221", kernel);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return (unsigned int)kernel->getHeight();
}

unsigned int
xil_kernel_get_key_x(XilKernel* kernel)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-221", kernel);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return (unsigned int)kernel->getKeyX();
}

unsigned int
xil_kernel_get_key_y(XilKernel* kernel)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-221", kernel);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return (unsigned int)kernel->getKeyY();
}


void
xil_kernel_get_values(XilKernel*   kernel, 
                      float*       data)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-131", kernel);

    kernel->lock();

    kernel->getValues(data);

    kernel->unlock();
}



void
xil_interpolation_table_get_values(XilInterpolationTable*   interp_table, 
                                   float*                   data)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-131", interp_table);

    interp_table->lock();

    interp_table->getValues(data);

    interp_table->unlock();
}

//
//  Dither Masks
//
XilDitherMask*
xil_dithermask_create(XilSystemState* system_state,
                      unsigned int    width,
                      unsigned int    height, 
                      unsigned int    nbands,
                      float*          data)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilDitherMask(width, height, nbands, data);
}

void
xil_dithermask_destroy(XilDitherMask* dmask)
{
    if(dmask == NULL) {
        return;
    }

    dmask->lock();

    //
    //  Destroying the dither mask will take care of releasing the lock.
    //
    dmask->destroy();
}

unsigned int
xil_dithermask_get_width(XilDitherMask* dmask)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-222", dmask);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return dmask->getWidth();
}

unsigned int
xil_dithermask_get_height(XilDitherMask* dmask)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-222", dmask);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return dmask->getHeight();
}

unsigned int
xil_dithermask_get_nbands(XilDitherMask* dmask)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-222", dmask);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return dmask->getNumBands();
}

XilDitherMask*
xil_dithermask_create_copy(XilDitherMask* dmask)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-222", dmask);

    dmask->lock();

    XilDitherMask* copy = (XilDitherMask*)dmask->createCopy();

    dmask->unlock();

    return copy;
}

void
xil_dithermask_get_values(XilDitherMask*   dithermask, 
                          float*           data)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-131", dithermask);

    dithermask->lock();

    dithermask->getValues(data);

    dithermask->unlock();
}


//
//  XilSel:  Structuring Element
//
XilSel*
xil_sel_create(XilSystemState* system_state,
               unsigned int    width,
               unsigned int    height, 
               unsigned int    keyx,
               unsigned int    keyy,
               unsigned int*   data)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilSel(width, height, keyx, keyy, data);
}

void
xil_sel_destroy(XilSel* sel)
{
    if(sel == NULL) {
        return;
    }

    sel->lock();

    //
    //  Destroying the image will take care of releasing the lock.
    //
    sel->destroy();
}

unsigned int
xil_sel_get_height(XilSel* sel)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-262", sel);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return sel->getHeight();
}

unsigned int
xil_sel_get_width(XilSel* sel)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-262", sel);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return sel->getWidth();
}

unsigned int
xil_sel_get_key_x(XilSel* sel)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-262", sel);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return sel->getKeyX();
}

unsigned int
xil_sel_get_key_y(XilSel* sel)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-262", sel);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return sel->getKeyY();
}

XilSel*
xil_sel_create_copy(XilSel* sel)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-262", sel);

    sel->lock();

    XilSel* copy = (XilSel*)sel->createCopy();

    sel->unlock();

    return copy;
}

void
xil_sel_get_values(XilSel*       sel, 
                   unsigned int* data)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-131", sel);

    sel->lock();

    sel->getValues(data);

    sel->unlock();
}

//
//  XilImage Object
//
//  Functions to Get and Set Image Attributes
//
int
xil_get_exported(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-207", image);

    image->lock();

    int exported = image->getExported();

    image->unlock();

    return exported;
}

Xil_boolean
xil_get_readable(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-207", image);

    image->lock();

    Xil_boolean readable = image->isReadable();

    image->unlock();

    return readable;
}

Xil_boolean
xil_get_writeable(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-207", image);

    image->lock();

    Xil_boolean writable = image->isWritable();

    image->unlock();

    return writable;
}

//
//  New binding with correct spelling...
//
Xil_boolean
xil_get_writable(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-207", image);

    image->lock();

    Xil_boolean writable = image->isWritable();

    image->unlock();

    return writable;
}

void
xil_set_storage_movement(XilImage* image,
                         XilStorageMovement move_flag)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->setStorageMovement(move_flag);

    image->unlock();
}

XilStorageMovement
xil_get_storage_movement(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_ALLOW_MOVE, "di-207", image);

    image->lock();

    XilStorageMovement movement = image->getStorageMovement();

    image->unlock();

    return movement;
} 

void
xil_set_roi(XilImage* image,
	    XilRoi*   roi)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->setRoi(roi);

    image->unlock();
}

unsigned int
xil_get_width(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-207", image);

    image->lock();

    unsigned int width = image->getWidth();

    image->unlock();

    return width;
}

unsigned int
xil_get_height(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-207", image);

    image->lock();

    unsigned int height = image->getHeight();

    image->unlock();

    return height;
}

float
xil_get_pixel_width(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-207", image);

    image->lock();

    float width = image->getPixelWidth();

    image->unlock();

    return width;
}

float
xil_get_pixel_height(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-207", image);

    image->lock();

    float height = image->getPixelHeight();

    image->unlock();

    return height;
}

void
xil_set_pixel_width(XilImage* image, float width)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->setPixelWidth(width);

    image->unlock();
}

void
xil_set_pixel_height(XilImage* image, float height)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->setPixelHeight(height);

    image->unlock();
}


unsigned int
xil_get_nbands(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-207", image);

    image->lock();

    unsigned int nbands = image->getNumBands();

    image->unlock();

    return nbands;
}

XilDataType
xil_get_datatype(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_BIT, "di-207", image);

    image->lock();

    XilDataType data_type = image->getDataType();

    image->unlock();

    return data_type;
}

void
xil_get_size(XilImage*     image,
             unsigned int* width,
             unsigned int* height)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->getSize(width, height);

    image->unlock();
}

void
xil_get_info(XilImage*     image,
             unsigned int* width,
             unsigned int* height,
             unsigned int* nbands,
             XilDataType*  datatype)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->getInfo(width, height, nbands, datatype);

    image->unlock();
}

float
xil_get_origin_x(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(0.0F, "di-207", image);

    image->lock();

    float x_origin = image->getOriginX();

    image->unlock();

    return x_origin;
}

float
xil_get_origin_y(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(0.0F, "di-207", image);

    image->lock();

    float y_origin = image->getOriginY();

    image->unlock();

    return y_origin;
}

void
xil_get_origin(XilImage* image,
               float*    x,
               float*    y)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->getOrigin(x, y);

    image->unlock();
}

void
xil_set_origin(XilImage* image,
               float     x,
               float     y)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->setOrigin(x, y);

    image->unlock();
}

XilImage*
xil_get_parent(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-207", image);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    return image->getParent();
}

void
xil_get_child_offsets(XilImage*     image,
                      unsigned int* offsetX,
		      unsigned int* offsetY,
                      unsigned int* offsetBand)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    //
    //  Don't need to lock because the object doesn't change and the
    //  object creation is protected.
    //
    image->getChildOffsets(offsetX, offsetY, offsetBand);
}

int
xil_export(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_FAILURE, "di-207", image);

    image->lock();

    //
    //  Indicate we want the semantics of not permitting device images to be
    //  exported through the API.
    //
    int status = image->exportStorage(FALSE);

    image->unlock();

    return status;
}

void
xil_import(XilImage*   image,
           Xil_boolean change_flag)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->import(change_flag);

    image->unlock();
}

Xil_boolean
xil_get_memory_storage(XilImage*           image,
                       XilMemoryStorage_C* storage)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-207", image);

    image->lock();

    Xil_boolean status =
        image->getExportedMemoryStorage((XilMemoryStorage*)storage);

    image->unlock();

    return status;
}

void
xil_get_tilesize(XilImage*     image,
                 unsigned int* tile_xsize,
                 unsigned int* tile_ysize)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->getExportedTileSize(tile_xsize, tile_ysize);

    image->unlock();
}

void
xil_set_tilesize(XilImage*    image,
                 unsigned int tile_xsize,
                 unsigned int tile_ysize)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->setExportedTileSize(tile_xsize, tile_ysize);

    image->unlock();
}

Xil_boolean
xil_get_tile_storage(XilImage*      image,
                     unsigned int   x,
                     unsigned int   y,
                     XilStorageAPI* storage)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-207", image);

    image->lock();

    Xil_boolean status = image->getExportedStorage(storage, x, y);

    image->unlock();

    return status;
}

Xil_boolean
xil_set_tile_storage(XilImage*      image,
                     XilStorageAPI* storage)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-207", image);

    image->lock();

    Xil_boolean status = image->setExportedTileStorage(storage);

    image->unlock();

    return status;
}

XilStorageAPI*
xil_get_storage_with_copy(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-207", image);

    image->lock();

    XilStorageAPI* storage = image->getExportedStorageWithCopy();

    image->unlock();

    return storage;
}

int
xil_set_storage_with_copy(XilImage*      image,
                          XilStorageAPI* storage)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_FAILURE, "di-207", image);

    image->lock();

    int status = image->setExportedStorageWithCopy(storage);

    image->unlock();

    return status;
}

void
xil_set_pixel(XilImage*    image,
	      unsigned int x,
	      unsigned int y,
	      float*       values)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->setPixel(x, y, values);

    image->unlock();
}

void
xil_get_pixel(XilImage*    image,
	      unsigned int x,
	      unsigned int y,
	      float*       values)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->getPixel(x, y, values);

    image->unlock();
}

Xil_boolean
xil_get_synchronize(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-207", image);

    image->lock();

    Xil_boolean status = image->getSynchronized();

    image->unlock();

    return status;
}
 
void
xil_set_synchronize(XilImage*   image,
                    Xil_boolean onoff)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->setSynchronized(onoff);

    image->unlock();
}

int
xil_set_attribute(XilImage* image,
                  char*     attribute,
                  void*     value)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_FAILURE, "di-207", image);

    image->lock();

    int status = image->setAttribute(attribute, value);

    image->unlock();

    return status;
}

int
xil_get_attribute(XilImage* image,
                  char*     attribute,
                  void**    value)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_FAILURE, "di-207", image);

    image->lock();

    int status = image->getAttribute(attribute, value);

    image->unlock();

    return status;
}

int
xil_set_device_attribute(XilImage* image,
                         char*     attribute,
                         void*     value)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_FAILURE, "di-207", image);

    image->lock();

    int status = image->setDeviceAttribute(attribute, value);

    image->unlock();

    return status;
}

int
xil_get_device_attribute(XilImage* image,
                         char*     attribute,
                         void**    value)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_FAILURE, "di-207", image);

    image->lock();

    int status = image->getDeviceAttribute(attribute, value);

    image->unlock();

    return status;
}

void
xil_set_colorspace(XilImage*      image,
		   XilColorspace* cspace)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->setColorspace(cspace);

    image->unlock();
}

//
//  XilImageType object
//
unsigned int
xil_imagetype_get_width(XilImageFormat* image_format)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-207", image_format);

    image_format->lock();

    unsigned int width = image_format->getWidth();

    image_format->unlock();

    return width;
}

unsigned int
xil_imagetype_get_height(XilImageFormat* image_format)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-207", image_format);

    image_format->lock();

    unsigned int height = image_format->getHeight();

    image_format->unlock();

    return height;
}

unsigned int
xil_imagetype_get_nbands(XilImageFormat* image_format)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-207", image_format);

    image_format->lock();

    unsigned int nbands = image_format->getNumBands();

    image_format->unlock();

    return nbands;
}

void
xil_imagetype_get_size(XilImageFormat* image_format,
                       unsigned int*   width,
                       unsigned int*   height)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image_format);

    image_format->lock();

    image_format->getSize(width, height);

    image_format->unlock();
}

XilDataType
xil_imagetype_get_datatype(XilImageFormat* image_format)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_BIT, "di-207", image_format);

    image_format->lock();

    XilDataType datatype = image_format->getDataType();

    image_format->unlock();

    return datatype;
}

void
xil_imagetype_get_info(XilImageFormat* image_format,
                       unsigned int*   width,
                       unsigned int*   height,
                       unsigned int*   nbands,
                       XilDataType*    datatype)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image_format);

    image_format->lock();

    image_format->getInfo(width, height, nbands, datatype);

    image_format->unlock();
}

//
//  XilDevice Object
//
XilDevice*
xil_device_create(XilSystemState* system_state,
                  char*           device_name)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilDevice(device_name);
}

//
//  This is here for backward compatibility -- the preferred function to use
//  is xil_device_set_attribute().
//
void
xil_device_set_value(XilDevice* device,
                     char*      attribute_name,
                     void*      attribute_value)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-344", device);

    device->lock();

    device->setAttribute(attribute_name,
                         attribute_value);
    device->unlock();
}

void
xil_device_set_attribute(XilDevice* device,
                         char*      attribute_name,
                         void*      attribute_value)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-344", device);

    device->lock();

    device->setAttribute(attribute_name,
                         attribute_value);
    device->unlock();
}

void
xil_device_destroy(XilDevice* device)
{
    if(device == NULL) {
        return;
    }

    device->lock();

    //
    //  Destroying the device will take care of releasing the lock.
    //
    device->destroy();
}

//
//  XilRoi Object Methods
//
XilRoi*
xil_roi_create(XilSystemState* state)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return state->createXilRoi();
}

void
xil_roi_destroy(XilRoi* roi)
{
    if(roi == NULL) {
        return;
    }

    roi->lock();

    //
    //  Destroying the ROI will take care of releasing the lock.
    //
    roi->destroy();
}

void
xil_roi_add_rect(XilRoi* roi,
                 long    x,
                 long    y,
		 long    width,
                 long    height)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-257", roi);

    roi->lock();

    roi->addRect((int)x, (int)y,
                 (unsigned int)width, (unsigned int)height);

    roi->unlock();
}

void
xil_roi_add_image(XilRoi*   roi,
		  XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-257", roi);
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    //
    //  TODO: 9/7/96 jlf  Maybe just a static lock?  Need to devise a method
    //                    for locking both the ROI and the Image. 
    //
    roi->lock();

    roi->addImage(image);

    roi->unlock();
}

XilImage*
xil_roi_get_as_image(XilRoi* roi)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-257", roi);

    roi->lock();

    XilImage* image = roi->getAsImage();

    roi->unlock();

    return image;
}

void
xil_roi_subtract_rect(XilRoi* roi,
		      long    x,
		      long    y,
		      long    width,
		      long    height)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-257", roi);

    roi->lock();

    roi->subtractRect((int)x, (int)y,
                      (unsigned int)width,(unsigned int)height);

    roi->unlock();
}

XilRoi*
xil_get_roi(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-207", image);

    image->lock();

    XilRoi* roi = image->getRoi();

    image->unlock();

    return roi;
}

XilRoi*	
xil_roi_intersect(XilRoi* roi1, 
		  XilRoi* roi2)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-187", roi1);
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-187", roi2);

    //
    //  TODO: 9/7/96 jlf  Maybe just a static lock?  Need to devise a method
    //                    for locking both ROIs.
    //
    return roi1->intersect(roi2);
}

XilRoi*	
xil_roi_unite(XilRoi* roi1, 
	      XilRoi* roi2)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-257", roi1);
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-257", roi2);

    //
    //  TODO: 9/7/96 jlf  Maybe just a static lock?  Need to devise a method
    //                    for locking both ROIs.
    //
    return roi1->unite(roi2);
}

XilRoi*	
xil_roi_translate(XilRoi* roi, 
		  int     x,
		  int     y)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-257", roi);

    roi->lock();

    XilRoi* translated_roi = roi->translate(x, y);

    roi->unlock();

    return translated_roi;
}

XilRoi*	
xil_roi_create_copy(XilRoi* roi)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-257", roi);

    roi->lock();

    XilRoi* copy = (XilRoi*)roi->createCopy();

    roi->unlock();

    return copy;
}

XilRoi*
xil_roi_scale(XilRoi* roi,
              float xscale,
              float yscale,
              float xorigin,
              float yorigin)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-257", roi);

    roi->lock();

    XilRoi* scaled_roi = roi->scale(xscale, yscale, xorigin, yorigin);

    roi->unlock();

    return scaled_roi;
}

XilRoi*
xil_roi_transpose(XilRoi* roi,
                  XilFlipType fliptype,
                  float xorigin,
                  float yorigin)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-257", roi);

    roi->lock();

    XilRoi* transposed_roi = roi->transpose(fliptype, xorigin, yorigin);

    roi->unlock();

    return transposed_roi;
}


#if defined(_XIL_HAS_X11WINDOWS) || defined(_WINDOWS)
Region
xil_roi_get_as_region(XilRoi* roi)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-257", roi);

    roi->lock();
    
    Region region = roi->getAsRegion();

    roi->unlock();

    return region;
}

void
xil_roi_add_region(XilRoi* roi,
                   Region region)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-257", roi);
    _XIL_TEST_FOR_NULL_PTR_VOID("di-301", region);

    roi->lock();

    roi->addRegion(region);

    roi->unlock();
}
#endif   /* _XIL_HAS_X11WINDOWS || _WINDOWS */

//
//  InterpolationTables
//
XilInterpolationTable*
xil_interpolation_table_create(XilSystemState* system_state,
                               unsigned int    kernel_size,
                               unsigned int    subsamples,
                               float*          data)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilInterpolationTable(kernel_size,
                                                     subsamples,
                                                     data);
}

XilInterpolationTable*	
xil_interpolation_table_create_copy(XilInterpolationTable* table)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-352", table);

    table->lock();

    XilInterpolationTable* copy = (XilInterpolationTable*)table->createCopy();

    table->unlock();

    return copy;
}


unsigned int
xil_interpolation_table_get_kernel_size(XilInterpolationTable* table)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-352", table);

    table->lock();

    unsigned int ksize = table->getKernelSize();

    table->unlock();

    return ksize;
}

unsigned int
xil_interpolation_table_get_subsamples(XilInterpolationTable* table)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-352", table);

    table->lock();

    unsigned int subsamples = table->getNumSubsamples();

    table->unlock();

    return subsamples;
}

float*
xil_interpolation_table_get_data(XilInterpolationTable* table)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-352", table);

    table->lock();

    unsigned int data_size =
        table->getKernelSize() *
        table->getNumSubsamples() *
        sizeof(float);

    float*       data_buf  = (float*)malloc(data_size);
    if(data_buf == NULL) {
        XIL_ERROR(table->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        table->unlock();
        return NULL;
    }

    xili_memcpy(data_buf, table->getData(), data_size);

    table->unlock();

    return data_buf;
}

void
xil_interpolation_table_destroy(XilInterpolationTable* table)
{
    if(table == NULL) {
        return;
    }

    table->lock();

    //
    //  Destroying the table will take care of releasing the lock.
    //
    table->destroy();
}

//
//  XilStorage Object (new for XIL 1.3)
//
XilStorageAPI*
xil_storage_create(XilSystemState* state,
                   XilImage*       image)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return state->createXilStorageAPI(image);
}

void
xil_storage_destroy(XilStorageAPI* storage)
{
    if(storage == NULL) {
        return;
    }

    storage->lock();

    //
    //  Destroying the storage object will take care of releasing the lock.
    //
    storage->destroy();
}

Xil_boolean
xil_storage_is_type(XilStorageAPI* storage,
                    XilStorageType target_type)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-383", storage);

    storage->lock();

    Xil_boolean status = storage->isType(target_type);

    storage->unlock();

    return status;
}

unsigned int
xil_storage_get_scanline_stride(XilStorageAPI* storage,
                                unsigned int   band_num)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-383", storage);

    storage->lock();

    unsigned int scanline_stride = storage->getScanlineStride(band_num);

    storage->unlock();

    return scanline_stride;
}

unsigned int
xil_storage_get_pixel_stride(XilStorageAPI* storage,
                             unsigned int   band_num)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-383", storage);

    storage->lock();

    unsigned int pixel_stride = storage->getPixelStride(band_num);

    storage->unlock();

    return pixel_stride;
}

unsigned int
xil_storage_get_band_stride(XilStorageAPI* storage)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-383", storage);

    storage->lock();

    unsigned int band_stride = storage->getBandStride();

    storage->unlock();

    return band_stride;
}

unsigned int
xil_storage_get_offset(XilStorageAPI* storage,
                       unsigned int   band_num)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-383", storage);

    storage->lock();

    unsigned int offset = storage->getOffset(band_num);

    storage->unlock();

    return offset;
}

void*
xil_storage_get_data(XilStorageAPI* storage,
                     unsigned int   band_num)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-383", storage);

    storage->lock();

    void* data = storage->getDataPtr(band_num);

    storage->unlock();

    return data;
}

void
xil_storage_get_coordinates(XilStorageAPI* storage,
                            unsigned int*  left_x,
                            unsigned int*  upper_y)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-383", storage);

    storage->lock();

    storage->getCoordinates(left_x,upper_y);

    storage->unlock();
}


void
xil_storage_set_scanline_stride(XilStorageAPI* storage,
                                unsigned int   band_num,
                                unsigned int   scanline_stride)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-383", storage);

    storage->lock();

    storage->setScanlineStride(scanline_stride, band_num);

    storage->unlock();
}

void
xil_storage_set_pixel_stride(XilStorageAPI* storage,
                             unsigned int   band_num,
                             unsigned int   pixel_stride)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-383", storage);

    storage->lock();

    storage->setPixelStride(pixel_stride, band_num);

    storage->unlock();
}

void
xil_storage_set_band_stride(XilStorageAPI* storage,
                            unsigned int   band_stride)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-383", storage);

    storage->lock();

    storage->setBandStride(band_stride);

    storage->unlock();
}

void
xil_storage_set_offset(XilStorageAPI* storage,
                       unsigned int   band_num,
                       unsigned int   offset)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-383", storage);

    storage->lock();

    storage->setOffset(offset, band_num);

    storage->unlock();
}

void
xil_storage_set_data(XilStorageAPI*             storage,
                     unsigned int               band_num,
                     void*                      data_ptr)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-383", storage);

    storage->lock();

    storage->setDataPtr(data_ptr, band_num);

    storage->unlock();
}

void
xil_storage_set_data_release(XilStorageAPI*        storage,
                             XilDataReleaseFuncPtr release_func,
                             void*                 user_args)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-383", storage);

    storage->lock();

    storage->setDataReleaseFunc(release_func, user_args);

    storage->unlock();
}

void
xil_storage_set_coordinates(XilStorageAPI* storage,
                            unsigned int   xcoord,
                            unsigned int   ycoord)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-383", storage);

    storage->setCoordinates(xcoord, ycoord);
}

//
//  Colorspace Object Methods
//
XilColorspace*
xil_colorspace_create(XilSystemState*   system_state,
                      XilColorspaceType type,
                      void*             data)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilColorspace(type, data);
}


void
xil_colorspace_destroy(XilColorspace* colorspace)
{
    if(colorspace == NULL) {
        return;
    }

    colorspace->lock();

    //
    //  Destroying the colorspace will take care of releasing the lock.
    //
    colorspace->destroy();
}

XilColorspaceType
xil_colorspace_get_type(XilColorspace* colorspace)
{
    return colorspace->getColorspaceType();
}

//
//  ColorspaceList Object Methods
//
XilColorspaceList*
xil_colorspacelist_create(XilSystemState* system_state,
                          XilColorspace** colorspace_array,
                          unsigned int    num_colorspaces)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilColorspaceList(colorspace_array,
                                                 num_colorspaces);
}

void
xil_colorspacelist_destroy(XilColorspaceList* colorspacelist)
{
    if(colorspacelist == NULL) {
        return;
    }

    colorspacelist->lock();

    //
    //  Destroying the colorspace list will take care of releasing the lock.
    //
    colorspacelist->destroy();
}


//
//  CIS Object Methods
//
XilCis*
xil_cis_create(XilSystemState* system_state, 
               char*           name)
{
    //
    //  XilSystemState is MT-SAFE
    //
    return system_state->createXilCis(name);
}

void
xil_cis_destroy(XilCis* cis)
{
    if(cis == NULL) {
        return;
    }

    cis->lock();

    //
    //  Destroying the CIS will take care of releasing the lock.
    //
    cis->destroy();
}

void
xil_cis_flush(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-117", cis);

    cis->lock();

    cis->flush();

    cis->unlock();
}

int	
xil_cis_set_attribute(XilCis* cis,
                      char*   attribute,
                      void*   value)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_FAILURE, "di-117", cis);

    cis->lock();

    int status = cis->setAttribute(attribute, value);

    cis->unlock();

    return status;
}

int	
xil_cis_get_attribute(XilCis* cis,
                      char*   attribute,
                      void**  value)
{
    _XIL_TEST_FOR_NULL_PTR(XIL_FAILURE, "di-117", cis);

    cis->lock();

    int status = cis->getAttribute(attribute, value);

    cis->unlock();

    return status;
}


int
xil_cis_has_data(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-117", cis);

    cis->lock();

    int status = cis->hasData();

    cis->unlock();

    return status;
}

Xil_boolean
xil_cis_has_frame(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-117", cis);

    cis->lock();

    Xil_boolean has_frame = cis->hasFrame();

    cis->unlock();

    return has_frame;
}

void*
xil_cis_get_bits_ptr(XilCis* cis,
                     int*    nbytes,
                     int*    nframes)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-117", cis);

    cis->lock();

    void* data = cis->getBitsPtr(nbytes, nframes);

    cis->unlock();

    return data;
}

void
xil_cis_sync(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-117", cis);

    cis->lock();

    cis->sync();

    cis->unlock();
}

void
xil_cis_reset(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-117", cis);

    cis->lock();

    cis->reset();

    cis->unlock();
}

void
xil_cis_put_bits(XilCis* cis,
                 int     nbytes,
                 int     nframes,
                 void*   data)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-117", cis);

    cis->lock();

    cis->putBits(nbytes, nframes, data);

    cis->unlock();
}

int
xil_cis_number_of_frames(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-117", cis);

    cis->lock();

    int frames = cis->numberOfFrames();

    cis->unlock();

    return frames;
}

char*
xil_cis_get_compressor(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-117", cis);

    cis->lock();

    char* compressor = cis->getCompressor();

    cis->unlock();

    return compressor;
}

char*
xil_cis_get_compression_type(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-117", cis);

    cis->lock();

    char* type = cis->getCompressionType();

    cis->unlock();

    return type;
}

int
xil_cis_get_random_access(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-117", cis);

    cis->lock();

    int random_access = cis->getRandomAccess();

    cis->unlock();

    return random_access;
}

int
xil_cis_get_start_frame(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-117", cis);

    cis->lock();

    int start_frame = cis->getStartFrame();

    cis->unlock();

    return start_frame;
}

int
xil_cis_get_read_frame(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-117", cis);

    cis->lock();

    int read_frame = cis->getReadFrame();

    cis->unlock();

    return read_frame;
}

int
xil_cis_get_write_frame(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-117", cis);

    cis->lock();

    int write_frame = cis->getWriteFrame();

    cis->unlock();

    return write_frame;
}

Xil_boolean
xil_cis_get_read_invalid(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-117", cis);

    cis->lock();

    int read_invalid = cis->getReadInvalid();

    cis->unlock();

    return read_invalid;
}

Xil_boolean
xil_cis_get_write_invalid(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-117", cis);

    cis->lock();

    int write_invalid = cis->getWriteInvalid();

    cis->unlock();

    return write_invalid;
}

Xil_boolean
xil_cis_get_autorecover(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(FALSE, "di-117", cis);

    cis->lock();

    Xil_boolean autorecover = cis->getAutorecover();

    cis->unlock();

    return autorecover;
}

void
xil_cis_set_autorecover(XilCis*     cis,
                        Xil_boolean on_off)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-117", cis);

    cis->lock();

    cis->setAutorecover(on_off);

    cis->unlock();
}

void
xil_cis_attempt_recovery(XilCis*      cis,
                         unsigned int nframes,
                         unsigned int nbytes)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-117", cis);

    cis->lock();

    cis->attemptRecovery(nframes, nbytes);

    cis->unlock();
}

XilImageFormat*
xil_cis_get_output_type(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-117", cis);

    cis->lock();

    XilImageFormat* format = cis->getOutputType();

    cis->unlock();

    return format;
}

XilImageFormat*
xil_cis_get_input_type(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-117", cis);

    cis->lock();

    XilImageFormat* format = cis->getInputType();

    cis->unlock();

    return format;
}

int
xil_cis_get_keep_frames(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-117", cis);

    cis->lock();

    int keep_frames = cis->getKeepFrames();

    cis->unlock();

    return keep_frames;
}

void
xil_cis_set_keep_frames(XilCis* cis,
                        int     keep)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-117", cis);

    cis->lock();

    cis->setKeepFrames(keep);

    cis->unlock();
}

int
xil_cis_get_max_frames(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(0, "di-117", cis);

    cis->lock();

    int max_frames = cis->getMaxFrames();

    cis->unlock();

    return max_frames;
}

void
xil_cis_set_max_frames(XilCis* cis,
                       int     max)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-117", cis);

    cis->lock();

    cis->setMaxFrames(max);

    cis->unlock();
}

void
xil_cis_put_bits_ptr(XilCis*                    cis,
                     int                        nbytes,
                     int                        nframes,
                     void*                      data,
                     XIL_FUNCPTR_DONE_WITH_DATA done_with_data)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-117", cis);

    cis->lock();

    cis->putBitsPtr(nbytes, nframes, data, done_with_data);

    cis->unlock();
}

void
xil_cis_seek(XilCis* cis,
             int     framenumber,
             int     relative_to)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-117", cis);

    cis->lock();

    cis->seek(framenumber, relative_to);

    cis->unlock();
}

//
// API bindings for naming XIL objects
//
char*
xil_get_name(XilImage* image)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-207", image);

    image->lock();

    char* name = image->getName();

    image->unlock();

    return name;
}

void
xil_set_name(XilImage* image,
	     char*     name)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->setName(name);

    image->unlock();
}

char*
xil_imagetype_get_name(XilImageFormat* imageformat)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-207", imageformat);

    imageformat->lock();

    char* name = imageformat->getName();

    imageformat->unlock();

    return name;
}

void
xil_imagetype_set_name(XilImageFormat* imageformat,
		       char*           name)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", imageformat);

    imageformat->lock();

    imageformat->setName(name);

    imageformat->unlock();
}

char*
xil_lookup_get_name(XilLookup* lookup)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-131", lookup);

    lookup->lock();

    char* name = lookup->getName();

    lookup->unlock();

    return name;
}

void
xil_lookup_set_name(XilLookup* lookup,
		    char*      name)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-131", lookup);

    lookup->lock();

    lookup->setName(name);

    lookup->unlock();
}

char*
xil_cis_get_name(XilCis* cis)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-117", cis);

    cis->lock();

    char* name = cis->getName();

    cis->unlock();

    return name;
}

void
xil_cis_set_name(XilCis* cis,
		 char*   name)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-117", cis);

    cis->lock();

    cis->setName(name);

    cis->unlock();
}

char*
xil_dithermask_get_name(XilDitherMask* dithermask) 
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-222", dithermask);

    dithermask->lock();

    char* name = dithermask->getName();

    dithermask->unlock();

    return name;
}

void
xil_dithermask_set_name(XilDitherMask* dithermask,
			char*          name)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-222", dithermask);

    dithermask->lock();

    dithermask->setName(name);

    dithermask->unlock();
}

char*
xil_kernel_get_name(XilKernel* kernel) 
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-221", kernel);

    kernel->lock();

    char* name = kernel->getName();

    kernel->unlock();

    return name;
}

void
xil_kernel_set_name(XilKernel* kernel,
		    char*      name)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-221", kernel);

    kernel->lock();

    kernel->setName(name);

    kernel->unlock();
}

char*
xil_sel_get_name(XilSel* sel)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-262", sel);

    sel->lock();

    char* name = sel->getName();

    sel->unlock();

    return name;
}

void
xil_sel_set_name(XilSel* sel,
		 char*   name)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-262", sel);

    sel->lock();

    sel->setName(name);

    sel->unlock();
}

char*
xil_roi_get_name(XilRoi* roi)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-257", roi);

    roi->lock();

    char* name = roi->getName();

    roi->unlock();

    return name;
}

void
xil_roi_set_name(XilRoi* roi,
                 char*   name)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-257", roi);

    roi->lock();

    roi->setName(name);

    roi->unlock();
}

char*
xil_histogram_get_name(XilHistogram* histogram)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-265", histogram);

    histogram->lock();

    char* name = histogram->getName();

    histogram->unlock();

    return name;
}

void
xil_histogram_set_name(XilHistogram* histogram,
		       char*         name)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-265", histogram);

    histogram->lock();

    histogram->setName(name);

    histogram->unlock();
}

char*
xil_storage_get_name(XilStorageAPI* storage)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-383", storage);

    storage->lock();

    char* name = storage->getName();

    storage->unlock();

    return name;
}

void
xil_storage_set_name(XilStorageAPI* storage,
                     char*          name)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-383", storage);

    storage->lock();

    storage->setName(name);

    storage->unlock();
}

char*
xil_colorspace_get_name(XilColorspace* colorspace)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-296", colorspace);

    colorspace->lock();

    char* name = colorspace->getName();

    colorspace->unlock();

    return name;
}

void
xil_colorspace_set_name(XilColorspace* colorspace,
                        char*          name)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-296", colorspace);

    colorspace->lock();

    colorspace->setName(name);

    colorspace->unlock();
}

char*
xil_colorspacelist_get_name(XilColorspaceList* colorspacelist)
{
    _XIL_TEST_FOR_NULL_PTR(NULL, "di-372", colorspacelist);

    colorspacelist->lock();

    char* name = colorspacelist->getName();

    colorspacelist->unlock();

    return name;
}

void
xil_colorspacelist_set_name(XilColorspaceList* colorspacelist,
                            char*              name)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-372", colorspacelist);

    colorspacelist->lock();

    colorspacelist->setName(name);

    colorspacelist->unlock();
}

//
//  Binding for aquiring objects by name -- XilSystemState is MT-SAFE
//
XilStorageAPI*
xil_storage_get_by_name(XilSystemState* state,
                        char*           name)
{
    return (XilStorageAPI*)state->getObjectByName(name, XIL_STORAGE);
}

XilKernel*
xil_kernel_get_by_name(XilSystemState* state,
                       char*           name)
{
    return (XilKernel*)state->getObjectByName(name, XIL_KERNEL);
}

XilLookup*
xil_lookup_get_by_name(XilSystemState* state,
                       char*           name)
{ 
    return (XilLookup*)state->getObjectByName(name, XIL_LOOKUP);
}
 
XilDitherMask*
xil_dithermask_get_by_name(XilSystemState* state,
                           char*           name)
{  
    return (XilDitherMask*)state->getObjectByName(name, XIL_DITHER_MASK);
}
  
XilSel*
xil_sel_get_by_name(XilSystemState* state,
                    char*           name)
{   
    return (XilSel*)state->getObjectByName(name, XIL_SEL);
}

XilHistogram*
xil_histogram_get_by_name(XilSystemState* state,
                          char*           name)
{    
    return (XilHistogram*)state->getObjectByName(name, XIL_HISTOGRAM);
}

XilImage*
xil_get_by_name(XilSystemState* state,
                char*           name)
{     
    return (XilImage*)state->getObjectByName(name, XIL_IMAGE);
}

XilImageFormat*
xil_imagetype_get_by_name(XilSystemState* state,
                          char*           name)
{        
    return (XilImageFormat*)state->getObjectByName(name, XIL_IMAGE_TYPE);
}

XilRoi*
xil_roi_get_by_name(XilSystemState* state,
                    char*           name)
{         
    return (XilRoi*)state->getObjectByName(name, XIL_ROI);
}

XilCis*
xil_cis_get_by_name(XilSystemState* state,
                    char*           name)
{          
    return (XilCis*)state->getObjectByName(name, XIL_CIS);
}

XilColorspace*
xil_colorspace_get_by_name(XilSystemState* state,
                           char*           name)
{
    return (XilColorspace*)state->getObjectByName(name, XIL_COLORSPACE);
}

XilColorspaceList*
xil_colorspacelist_get_by_name(XilSystemState* state,
                               char*           name)
{
    return (XilColorspaceList*)state->getObjectByName(name,
                                                      XIL_COLORSPACE_LIST);
}

//
//  XilError Object and Error Handling -- XilSystemState and XilError handler
//  is MT-SAFE 
//
int
xil_install_error_handler(XilSystemState* system_state,
                          Xil_boolean     (*error_func)(XilError*))
{
    return system_state->installErrorHandler(error_func);
}

void
xil_remove_error_handler(XilSystemState* system_state,
                         Xil_boolean     (*error_func)(XilError*))
{
    system_state->removeErrorHandler(error_func);
}

Xil_boolean
xil_call_next_error_handler(XilError* error)
{
    return error->callNextErrorHandler();
}

Xil_boolean
xil_default_error_handler(XilError* error)
{
    return XiliDefaultErrorHandler(error);
}

//
//  XilError Object Methods
//
char*
xil_error_get_string(XilError* error)
{
    return (char*)error->getString();
}

char*
xil_error_get_id(XilError* error)
{
    return (char*)error->getId();
}

XilErrorCategory
xil_error_get_category(XilError* error)
{
    return error->getErrorCategory();
}

char*
xil_error_get_category_string(XilError* error)
{
    return (char*)error->getErrorCategoryString();
}

Xil_boolean
xil_error_get_primary(XilError* error)
{
    return error->getPrimaryFlag();
}

Xil_boolean
xil_error_is_warning(XilError* error)
{
    return error->isWarning();
}

XilObject*
xil_error_get_object(XilError* error)
{
    return error->getObject();
}

char*
xil_error_get_location(XilError* error)
{
    return (char*)error->getLocation();
}

void
xil_object_get_error_string(XilObject* object,
                            char*      string,
                            int        string_size)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-268", object);

    object->getErrorString(string, string_size);
}

void
xil_set_data_supply_routine(XilImage*             image,
                            XilDataSupplyFuncPtr  supply_func,
                            void*                 user_args)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    image->setDataSupplyFunc(supply_func, user_args);

    image->unlock();
}

void
xil_set_memory_storage(XilImage*           image,
                       XilMemoryStorage_C* storage)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-207", image);

    image->lock();

    XilMemoryStorage core_storage;

    XilDataType      datatype = image->getDataType();

    switch (datatype) {
      case XIL_BIT:
        core_storage.bit.data            = storage->bit.data;
        core_storage.bit.scanline_stride =
            (unsigned short)storage->bit.scanline_stride;
        core_storage.bit.band_stride     = storage->bit.band_stride;
        core_storage.bit.offset          = storage->bit.offset;
        core_storage.bit.start_data      = NULL;
        break;

      case XIL_BYTE:
        core_storage.byte.data            = storage->byte.data;
        core_storage.byte.scanline_stride = storage->byte.scanline_stride;
        core_storage.byte.pixel_stride    =
            (unsigned short)storage->byte.pixel_stride;
        core_storage.byte.start_data      = NULL;
        break;

      case XIL_SHORT:
        core_storage.shrt.data            = storage->shrt.data;
        core_storage.shrt.scanline_stride = storage->shrt.scanline_stride;
        core_storage.shrt.pixel_stride    =
            (unsigned short)storage->shrt.pixel_stride;
        core_storage.shrt.start_data = NULL;
        break;

      default:
        XIL_ERROR(_XILI_GET_STATE(image), XIL_ERROR_USER, "di-371", TRUE);
        image->unlock();
        return;
    }

    image->setExportedMemoryStorage(&core_storage);

    image->unlock();
}

} // Matching for extern "C"
