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
//  File:       _XilDeviceManagerCompression.hh
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:21:35, 03/10/00
//
//  Description:
//	
//   Abstract class which provides the entry point for the 
//   dynamic loading of a codec-specific compression handler 
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)_XilDeviceManagerCompression.hh	1.7\t00/03/10  "

#ifndef XILDEVICEMANAGERCOMPRESSION_H
#define XILDEVICEMANAGERCOMPRESSION_H

#include "_XilCis.hh"
#include "_XilDeviceManager.hh"
#include "_XilDeviceCompression.hh"

typedef void (XilDeviceCompression::*setAttrFunc)(void * value);
typedef void*  (XilDeviceCompression::*getAttrFunc)();

#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES
#include "XilDeviceManagerCompressionPrivate.hh"
#undef _XIL_PRIVATE_INCLUDES
#endif

class XilDeviceManagerCompression : public XilDeviceManager {
public:
    //
    //  The create() function lives in every compression pipeline and
    //    constructs a class derived from XilDeviceMangerCompression.
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
    static XilDeviceManagerCompression* create(unsigned int  libxil_gpi_major,
                                               unsigned int  libxil_gpi_minor,
                                               unsigned int* devhandler_gpi_major,
                                               unsigned int* devhandler_gpi_minor);

    //
    // Create the specific deviceCompression object
    // Pure virtual - must be implemented in subclass
    //
    virtual XilDeviceCompression* constructNewDevice(XilCis* cis) =0;

    void                          destroy();

    XilStatus   getAttr(XilDeviceCompression* xdc, 
                        const char*           name, 
                        void**                value);

    XilStatus   setAttr(XilDeviceCompression* xdc, 
                        const char*           name, 
                        void*                 value);

    char* getCompressor();
    char* getCompressionType();

protected:
    //
    // Base class constructor/destructor
    //
            XilDeviceManagerCompression(char *cname, char *ctype);
    virtual ~XilDeviceManagerCompression();

    void registerAttr(char* name, setAttrFunc set, getAttrFunc get);

    Xil_boolean isOK();

    //
    // Derived classes will update this flag
    //
    Xil_boolean isOKFlag;

private:
    char* compressor;
    char* compression_type;
//
// This must be a void* bechase we want to hide the definition of
// the HashTable Class which implements the attrTable
//
// TODO: Need something more basic. This is a hack.
//
    void* attrTable;

    void*   _extra_data[256];
};

#endif // XILDEVICEMANAGERCOMPRESSION_H
