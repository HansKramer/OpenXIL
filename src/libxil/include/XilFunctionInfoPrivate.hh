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
//  File:	XilFunctionInfoPrivate.hh
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:21:56, 03/10/00
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
#pragma ident	"@(#)XilFunctionInfoPrivate.hh	1.12\t00/03/10  "

#ifdef _XIL_PRIVATE_INCLUDES

#include <stdio.h>

#include "XiliList.hh"

class XiliDirection {
public:
    XilDirection          dir;
    int                   opNumber;

    int                   operator== (const XiliDirection& rval) const
    {
        return (dir == rval.dir) && (opNumber == rval.opNumber);
    }
};

//
//  Mark which type of function the info contains...
//
typedef enum {
    XILI_COMPUTE_FUNC,
    XILI_IO_FUNC,
    XILI_CODEC_FUNC
} XiliFuncType;

#endif // _XIL_PRIVATE_INCLUDES

#ifdef _XIL_PRIVATE_DATA

public:
    //
    //  Constructor/Destructor
    //
                                     XilFunctionInfo();

                                     ~XilFunctionInfo();

    XiliList<XiliDirection>*         getDirectionList()
    {
        return directionList;
    }

    void                             setDeleteDirectionList(Xil_boolean flag)
    {
        deleteDirectionList = flag;
    }

    XilComputeFunctionPtr            getComputeFunction()
    {
        return computeFunc;
    }

    XilIOFunctionPtr                 getIOFunction()
    {
        return ioFunc;
    }

    XilDeviceIO*                     getIODevice()
    {
        return ioDevice;
    }

    XilCodecFunctionPtr              getCodecFunction()
    {
        return codecFunc;
    }

    XilDeviceCompression*            getCodecDevice()
    {
        return codecDevice;
    }

    XilComputePreprocessFunctionPtr  getComputePreFunction()
    {
        return computePreFunc;
    }

    XilComputePostprocessFunctionPtr getComputePostFunction()
    {
        return computePostFunc;
    }

    XilIOPreprocessFunctionPtr       getIOPreFunction()
    {
        return ioPreFunc;
    }

    XilIOPostprocessFunctionPtr      getIOPostFunction()
    {
        return ioPostFunc;
    }

    XilCodecPreprocessFunctionPtr    getCodecPreFunction()
    {
        return codecPreFunc;
    }

    XilCodecPostprocessFunctionPtr   getCodecPostFunction()
    {
        return codecPostFunc;
    }

    const char*                      getFunctionName()
    {
        return functionName;
    }

public:
    XilComputePreprocessFunctionPtr  computePreFunc;
    XilComputeFunctionPtr            computeFunc;
    XilComputePostprocessFunctionPtr computePostFunc;
    XilDeviceManagerCompute*         computeDevice;

    XilIOPreprocessFunctionPtr       ioPreFunc;
    XilIOFunctionPtr                 ioFunc;
    XilIOPostprocessFunctionPtr      ioPostFunc;
    XilDeviceIO*                     ioDevice;
    
    XilCodecPreprocessFunctionPtr    codecPreFunc;
    XilCodecFunctionPtr              codecFunc;
    XilDeviceCompression*            codecDevice;
    XilCodecPostprocessFunctionPtr   codecPostFunc;

    const char*                      functionName;

    XiliList<XiliDirection>*         directionList;
    Xil_boolean                      deleteDirectionList;
    unsigned int                     numOpsInMolecule;
#endif // _XIL_PRIVATE_DATA

