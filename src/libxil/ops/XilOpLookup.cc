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
//  File:	XilOpLookup.cc
//  Project:	XIL
//  Revision:	1.17
//  Last Mod:	10:07:16, 03/10/00
//
//  Description:
//	XilOp object for the Lookup operation.
//	Derived from XilOpPoint. This op differs
//	from other "point" ops in that it involves
//      an auxilaiary object, the XilLookup object.
//      This contents of the Lookup object must be checked
//      for validity.
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilOpLookup.cc	1.17\t00/03/10  "

//
//  System Includes
//
#include <stdlib.h>
#include <string.h>

//
//  XIL C++ Includes
//
#include <xil/xilGPI.hh>

//
//  Local C++ Includes
//
#include "XilOpPoint.hh"
#include "XiliUtils.hh"

class XilOpLookup : public XilOpPoint {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    
protected:
    XilOpLookup(XilOpNumber op_num);
    virtual ~XilOpLookup();
};

#include <stdio.h>

XilOp*
XilOpLookup::create(char* func_name,        //  function_name
                    void* args[],
                    int   count)
{
    count = 0;                               // to remove comp. warning

    XilImage*  src    = (XilImage*)args[0];
    XilImage*  dst    = (XilImage*)args[1];
    XilLookup* lookup = (XilLookup*)args[2];
    
    //
    //  Check for NULL images.
    //
    if(dst == NULL) {
        XIL_ERROR(lookup->getSystemState(), XIL_ERROR_USER, "di-207", TRUE);
        return NULL;
    }
    if(src == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-207", TRUE);
        return NULL;
    }

    //
    //  Check input
    //
    if(lookup == NULL) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-131", TRUE);
        return NULL;
    }
    
    //
    //  Check for invalid src image.
    //
    if(src->isValid() == FALSE) {
        XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-327", TRUE);
        return NULL;
    }

    //
    //  Check the datatype compatibility of images with the lookup.
    //
    if(src->getDataType() != lookup->getInputDataType()) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-179", TRUE);
        return NULL;
    }
    
    if(dst->getDataType() != lookup->getOutputDataType()) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-180", TRUE);
        return NULL;
    }

    //
    //  Check the number of bands in the lookup with the number of
    //  bands for our images.
    //
    if(src->getNumBands() != lookup->getInputNBands()) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-181", TRUE);
        return NULL;
    }

    if(dst->getNumBands() != lookup->getOutputNBands()) {
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_USER, "di-299", TRUE);
        return NULL;
    }

    //
    //  There are multiple op names for lookup.  The first is the original
    //  lookup which is 1->N and is just "lookup".  The second is the
    //  multi-banded lookups which are N->N and is "lookup_n_to_n"
    //
    char*       base_name;
    XilOpCache* op_cache;
    if(lookup->getInputNBands() > 1) {
        static XilOpCache  lookup_n_to_n_op_cache(1);
        
        base_name = "lookup_n_to_n";
        op_cache  = &lookup_n_to_n_op_cache;
    } else {
        static XilOpCache  lookup_op_cache(1);
        
        base_name = "lookup";
        op_cache  = &lookup_op_cache;
    }

    //
    // Use the cast version to check the op cache as
    // image types can vary.
    //
    XilOpNumber	op_number;
    if((op_number = xili_check_op_cache_cast(base_name, op_cache, dst, src)) == -1) {
	return NULL;
    }

    //
    //  Create the op and set its arguments.
    //
    XilOpLookup* op = new XilOpLookup(op_number);
    if(op == NULL) {
	XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }	

    //
    // Copy the lookup table. Make a special case for single-band
    // combined lookups, so that a LookupSingle is supplied.
    //
    XilLookup* op_lut;
    if(lookup->getLookupType() == XIL_LOOKUP_COMBINED && 
       lookup->getInputNBands() == 1) {
        op_lut = (XilLookupSingle*)
                 ((XilLookupCombined*)lookup)->getBandLookup(0)->aquireDefRef(op);
    } else {
        op_lut = (XilLookup*)lookup->aquireDefRef(op);
    }
    if(op_lut == NULL) {
	op->destroy();
        XIL_ERROR(dst->getSystemState(), XIL_ERROR_RESOURCE, "di-1", FALSE);
	return NULL;
    }	
				    
    op->setSrc(1, src);
    op->setDst(1, dst);
    op->setParam(1, op_lut, XIL_RELEASE_REF);

    return op;
}

XilOpLookup::XilOpLookup(XilOpNumber op_num) : XilOpPoint(op_num) { ; }
XilOpLookup::~XilOpLookup() { }
