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
//  File:	_XilFunctionInfo.hh
//  Project:	XIL
//  Revision:	1.20
//  Last Mod:	10:22:00, 03/10/00
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
#pragma ident	"@(#)_XilFunctionInfo.hh	1.20\t00/03/10  "

#ifndef _XIL_COMPUTE_FUNCTION_INFO_HH
#define _XIL_COMPUTE_FUNCTION_INFO_HH

#include "_XilDefines.h"
#include "_XilClasses.hh"
#include "_XilGPIDefines.hh"

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilFunctionInfoPrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

//
//  Declare the assorted compute function pointers as typedefs for simpler
//  referencing.
//

//
//  Compute routine implementation function.
//
typedef
XilStatus
(XilDeviceManagerCompute::*XilComputeFunctionPtr)(XilOp*       op,
                                                  unsigned int op_count,
                                                  XilRoi*      roi,
                                                  XilBoxList*  bl);

//
//  I/O device operation implementation function.
//
typedef
XilStatus
(XilDeviceIO::*XilIOFunctionPtr)(XilOp*       op,
                                 unsigned int op_count,
                                 XilRoi*      roi,
                                 XilBoxList*  bl);

//
//  Compression device operation implementation function.
//
typedef
XilStatus
(XilDeviceCompression::*XilCodecFunctionPtr)(XilOp*       op,
                                             unsigned int op_count,
                                             XilRoi*      roi,
                                             XilBoxList*  bl);

//
//   Pre-processing routine for compute functions.  This function is optional
//     and will be called before any threads are started for a computation.
//     It is for setting up tables, potential simple determination of ability
//     to process the operation, etc.  If it returns XIL_FAILURE, then the XIL
//     core will not call this compute device to implement the operation.  The
//     compute routine can set the compute_data pointer which can be aquired
//     from the XilOp by the compute routines.
//
//   The final argument should be set to a unique number that associates this
//     preprocess routine with a particular compute function in the device.
//     This is for the case where there are multiple implementations of the
//     same operation in a single compute device.  The number is set to zero
//     by default and can be changed by the preprocess routine.  If the number
//     is set to something other than zero, it must be passed into the
//     XilOp::getComputeData() method in order to retrieve the correct parcel
//     of data.  The number only needs to be changed if there are multiple
//     functions implementing the same operation within a compute device --
//     for example two convolve routines each with their own preprocess
//     routines.
//
typedef
XilStatus
(XilDeviceManagerCompute::*XilComputePreprocessFunctionPtr)(XilOp*        op,
                                                            unsigned int  op_count,
                                                            XilRoi*       roi,
                                                            void**        pre_process_data,
                                                            unsigned int* func_ident);

//
//  Post-processing routine for cleanup of any data allocated in the
//    pre-processing routine.
//
typedef
XilStatus
(XilDeviceManagerCompute::*XilComputePostprocessFunctionPtr)(XilOp* op,
                                                             void*  pre_process_data);

//
//  Pre/Post-processing routines I/O devices.
//
typedef
XilStatus
(XilDeviceIO::*XilIOPreprocessFunctionPtr)(XilOp*        op,
                                           unsigned int  op_count,
                                           XilRoi*       roi,
                                           void**        pre_process_data,
                                           unsigned int* func_ident);

typedef
XilStatus
(XilDeviceIO::*XilIOPostprocessFunctionPtr)(XilOp* op,
                                            void*  pre_process_data);

//
//  Pre/Post-processing routines Codec devices.
//
typedef
XilStatus
(XilDeviceCompression::*XilCodecPreprocessFunctionPtr)(XilOp*        op,
                                                       unsigned int  op_count,
                                                       XilRoi*       roi,
                                                       void**        pre_process_data,
                                                       unsigned int* func_ident);

typedef
XilStatus
(XilDeviceCompression::*XilCodecPostprocessFunctionPtr)(XilOp* op,
                                                        void*  pre_process_data);


//
//  This class containes function-specific options.  It is set when adding a
//    function via addFunction() and is used for a compute device to alter the
//    XIL core's behavior when calling their compute routine.
//
//  In addition, this is the class which the compute device uses to provide the
//    description of a multi-branch molecule.
//
class XilFunctionInfo {
public:
    //
    //  Static create function which is used by the derived compute device
    //    routines to construct a new version of this class.
    //
    static XilFunctionInfo*  create();

    //
    //  Destroy this object...
    //
    void       destroy();
    
    //
    //  Set the pre-process function which (if set) will be called prior to
    //    each compute operation being executed.  Basically, this is called
    //    once per image and once per this type of operation in the DAG.
    //
    void       setPreprocessFunction(XilComputePreprocessFunctionPtr func);

    //
    //  Set the actual compute function which performs the operation described
    //    in this class.  The operation is described in a sequence of steps by
    //    using the describeOp method.  This function is called many times to
    //    operate on portions of an image.
    //
    void       setFunction(XilComputeFunctionPtr func,
                           const char*           name = NULL);

    //
    //  Set the post-process function while (if set) will be called after an
    //    entire compute operation has completed.  So, after the core has
    //    the comptute function (potentially many times) to complete the
    //    operation, this function is called to finish anything that is left.
    //
    void       setPostprocessFunction(XilComputePostprocessFunctionPtr func);

    //
    //  For I/O Devices
    //
    void       setFunction(XilIOFunctionPtr func,
                           const char*      name = NULL);
    void       setDevice(XilDeviceIO* device);
    void       setPreprocessFunction(XilIOPreprocessFunctionPtr func);
    void       setPostprocessFunction(XilIOPostprocessFunctionPtr func);

    //
    //  For Compression Devices
    //
    void       setFunction(XilCodecFunctionPtr func,
                           const char*         name = NULL);
    void       setDevice(XilDeviceCompression* device);
    void       setPreprocessFunction(XilCodecPreprocessFunctionPtr func);
    void       setPostprocessFunction(XilCodecPostprocessFunctionPtr func);

    //
    //  This is the method the derived compute device describes the operation
    //    to be performed by the function given in setComputeFunction.  It is
    //    a sequence of directions beginning at the base of the operation.
    //    So, atomic operations call this once where molecules will call this
    //    multiple times to describe the topography and operations that make
    //    up the molecule.
    //
    XilStatus  describeOp(XilDirection      dir,
                          unsigned int      branch_number,
                          const char*       operation_name);

    XilStatus  describeOp(XilDirection      dir,
                          Xil_boolean       leave_marker_flag);


private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilFunctionInfoPrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#endif
};

#endif // _XIL_COMPUTE_FUNCTION_INFO_HH
