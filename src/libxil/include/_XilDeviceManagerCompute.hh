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
//  File:	_XilDeviceManagerCompute.hh
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:21:22, 03/10/00
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
#pragma ident	"@(#)_XilDeviceManagerCompute.hh	1.10\t00/03/10  "

#ifndef _XIL_DEVICE_MANAGER_COMPUTE_HH
#define _XIL_DEVICE_MANAGER_COMPUTE_HH

//
//  C++ Includes
//
#include "_XilDefines.h"
#include "_XilClasses.hh"
#include "_XilGlobalState.hh"
#include "_XilDeviceManager.hh"

//
//  Private Includes
//
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES
    
#include "XilDeviceManagerComputePrivate.hh"
    
#undef  _XIL_PRIVATE_INCLUDES
#endif

class XilDeviceManagerCompute : public XilDeviceManager {
public:
    //
    //  The create() function lives in every compute pipeline and constructs
    //    a class derived from XilDeviceMangerCompute.
    //
    //  XIL provides the pipeline with the highest major and minor version
    //  numbers of the GPI it supports.  The compute pipeline is expected to
    //  fail if the version is not one that is supported by the pipeline.  For
    //  example, there is a mismatch in the major version numbers or the minor
    //  version is lower than the one required by the pipeline.
    //
    //  At the same time, the compute pipeline is expected to provide XIL with
    //  the highest version of the GPI it supports.  XIL may decide not to
    //  load the pipeline or it may decide to alter its behavior in order to
    //  support an older version of the interface.
    //
    static XilDeviceManagerCompute* create(unsigned int  libxil_gpi_major,
                                           unsigned int  libxil_gpi_minor,
                                           unsigned int* devhandler_gpi_major,
                                           unsigned int* devhandler_gpi_minor);

protected:
    //
    //  Base Constructor/Destructor
    //
                                    XilDeviceManagerCompute();
    virtual                         ~XilDeviceManagerCompute();

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilDeviceManagerComputePrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#endif

    //
    //  The priority of this compute device.  This is set from the OWconfig
    //  file and indicates an ordering to the compute routines.  Higher
    //  priority compute routines are called first.
    //
    unsigned int                    priority;

    //
    //  Extra data members which can be replaced with different data in future
    //    versions without breaking the derived classes.
    //
    void*                           _extra_data[256];
};

#endif  // _XIL_DEVICE_MANAGER_COMPUTE_HH
