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
//  File:	XilFunctionInfo.cc
//  Project:	XIL
//  Revision:	1.23
//  Last Mod:	10:08:41, 03/10/00
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
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilFunctionInfo.cc	1.23\t00/03/10  "


#include "_XilDefines.h"
#include "_XilFunctionInfo.hh"
#include "_XilGlobalState.hh"

#ifdef DEBUG
#include "XiliProcessEnv.hh"
#endif

XilFunctionInfo::XilFunctionInfo()
{
    computeFunc         = (XilComputeFunctionPtr)NULL;
    computePreFunc      = (XilComputePreprocessFunctionPtr)NULL;
    computePostFunc     = (XilComputePostprocessFunctionPtr)NULL;

    ioDevice            = NULL;
    ioFunc              = (XilIOFunctionPtr)NULL;
    ioPreFunc           = (XilIOPreprocessFunctionPtr)NULL;
    ioPostFunc          = (XilIOPostprocessFunctionPtr)NULL;

    codecDevice         = NULL;
    codecFunc           = (XilCodecFunctionPtr)NULL;
    codecPreFunc        = (XilCodecPreprocessFunctionPtr)NULL;
    codecPostFunc       = (XilCodecPostprocessFunctionPtr)NULL;

    functionName        = NULL;
    directionList       = new XiliList<XiliDirection>;
    if(directionList == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
    }
}

XilFunctionInfo::~XilFunctionInfo()
{
    delete directionList;
}

XilFunctionInfo*
XilFunctionInfo::create()
{
    return new XilFunctionInfo;
}

void
XilFunctionInfo::destroy()
{
    delete this;
}

//
//  Compute Device Managers
//
void
XilFunctionInfo::setFunction(XilComputeFunctionPtr func,
                             const char*           name)
{
    computeFunc  = func;
    functionName = name;
}

void
XilFunctionInfo::setPreprocessFunction(XilComputePreprocessFunctionPtr func)
{
    computePreFunc  = func;
}

void
XilFunctionInfo::setPostprocessFunction(XilComputePostprocessFunctionPtr func)
{
    computePostFunc = func;
}

//
//  I/O Devices
//
void
XilFunctionInfo::setFunction(XilIOFunctionPtr func,
                             const char*      name)
{
    ioFunc       = func;
    functionName = name;
}

void
XilFunctionInfo::setPreprocessFunction(XilIOPreprocessFunctionPtr func)
{
    ioPreFunc  = func;
}

void
XilFunctionInfo::setPostprocessFunction(XilIOPostprocessFunctionPtr func)
{
    ioPostFunc = func;
}

void
XilFunctionInfo::setDevice(XilDeviceIO* device)
{
    ioDevice = device;
}

//
//  Codec Devices
//
void
XilFunctionInfo::setFunction(XilCodecFunctionPtr func,
                             const char*         name)
{
    codecFunc    = func;
    functionName = name;
}

void
XilFunctionInfo::setPreprocessFunction(XilCodecPreprocessFunctionPtr func)
{
    codecPreFunc  = func;
}

void
XilFunctionInfo::setPostprocessFunction(XilCodecPostprocessFunctionPtr func)
{
    codecPostFunc = func;
}


void
XilFunctionInfo::setDevice(XilDeviceCompression* device)
{
    codecDevice = device;
}

//
//  Used for describing multi-branch molecules which are not supported in
//  XIL 1.3.
//
XilStatus
XilFunctionInfo::describeOp(XilDirection,
                            Xil_boolean)
{
    return XIL_FAILURE;
}

XilStatus
XilFunctionInfo::describeOp(XilDirection      dir,
                            unsigned int      branch_number,
                            const char*       operation_name)
{
    //
    //  Check to make sure it's a valid branch_number.  Multi-branch molecules
    //  are not supported in XIL 1.3 so anything other than "1" is an error.
    //
    if(branch_number != 1) {
        XIL_ERROR_WITH_ARG(NULL, XIL_ERROR_SYSTEM, "di-386",
                           TRUE, (void*)branch_number);
        return XIL_FAILURE;
    }
    
    //
    //  Insert (or potentially just lookup) the compute Op number as a new
    //    entry in the table containing the operation numbers.  If it is
    //    already there, then the existing entry is returned.
    //
    XilOpNumber op_num = XilGlobalState::theXGS->insertComputeOp(operation_name);

    //
    //  Create and fill in a new entry.
    //
    XiliDirection* new_dle = new XiliDirection;

    new_dle->dir           = dir;
    new_dle->opNumber      = op_num;

    directionList->append(new_dle);

    return XIL_SUCCESS;
}
