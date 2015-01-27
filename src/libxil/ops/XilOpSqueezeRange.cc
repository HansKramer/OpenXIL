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
//  File:	XilOpSqueezeRange.cc
//  Project:	XIL
//  Revision:	1.16
//  Last Mod:	10:07:23, 03/10/00
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
#pragma ident	"@(#)XilOpSqueezeRange.cc	1.16\t00/03/10  "

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>		// For XIL_MAXFLOAT

#include <xil/xilGPI.hh>
#include "XilOpDataCollect.hh"
#include "XiliOpUtils.hh"

class XilOpSqueezeRange : public XilOpDataCollect {
public:
    static XilOp* create(char* function_name,
                         void* args[],
                         int count);
    virtual XilStatus	vReportResults(void* results[]);
    virtual XilStatus	completeResults();
    
protected:
    // Constructor takes in the datatype of the image and
    // the pointer to return the created lookup table
    XilOpSqueezeRange(XilOpNumber op_num, XilDataType datatype, 
		      XilLookup** lut);
    virtual ~XilOpSqueezeRange();

private:
    // Used to lock update of values
    XilMutex		mutex;

    // Datatype of the image
    XilDataType		datatype;

    // Lut table to be returned to the user
    XilLookup**		user_lut;

    // Data type of the image
    int	                imax;
    int	                imin;
    Xil_unsigned8	*flags;
};

XilOp*
XilOpSqueezeRange::create(char  function_name[], 
			  void* args[], 
			  int)
{
    //
    //  There is no dst image, but xili_verify_op_args expects there
    //  to be one, 1.2 did it this way
    //
    XilImage*           src = (XilImage*)args[0];
    static XilOpCache	squeeze_range_op_cache;
    XilOpNumber		opnum;
    if((opnum = xili_verify_op_args(function_name, &squeeze_range_op_cache,
                                    src, src)) == -1) {
        return NULL;
    }

    //
    //  Only works for 1-banded images
    //
    if(src->getNumBands() != 1) {
 	XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-211", TRUE);
	return NULL;
    }

    //
    // This operation only works for BIT, BYTE and SHORT images
    //
    XilDataType   dtype = src->getDataType();
    if((dtype != XIL_BIT) && (dtype != XIL_BYTE) && (dtype != XIL_SHORT) &&
       (dtype != XIL_UNSIGNED_4) && (dtype != XIL_SIGNED_8) &&
       (dtype != XIL_UNSIGNED_16)) {
 	XIL_OBJ_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-434", TRUE, src);
	return NULL;
    }

    //
    //  Now create the op and set the op args
    //
    XilOpSqueezeRange*	op =
        new XilOpSqueezeRange(opnum, dtype, (XilLookup**)args[1]);
    if(op == NULL) {
	XIL_ERROR(src->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }
    
    op->setSrc(1, src);

    // Return the op
    return op;
}

// Constructor
XilOpSqueezeRange::XilOpSqueezeRange(XilOpNumber op_num,
				     XilDataType dtype,
				     XilLookup** lut) : XilOpDataCollect(op_num)
{
    // Store the datatype of the image
    datatype = dtype;
    flags    = NULL;
    user_lut = lut;
    imax     = 0;
    imin     = 0;
}

// Destructor
XilOpSqueezeRange::~XilOpSqueezeRange() { }

//
// Replace the XilOp virtual function
// The arguments expected are
//	unsigned int*	imin;
//	unsigned int*	imax;
//	Xil_unsigned8*	flags;
//
XilStatus
XilOpSqueezeRange::vReportResults(void* results[])
{
    int			i;
    int*	        result_imin;
    int*	        result_imax;
    Xil_unsigned8*	result_flags;

    mutex.lock();

    //
    // Extract the data the compute routine passed in
    // We need to get this first so we can initialize
    // imax and imin first time in.
    //
    result_imin  = (int*)results[0];
    result_imax  = (int*)results[1];
    result_flags = (Xil_unsigned8*)results[2];

    //
    //  Create the local flags area on the first pass
    //
    if(flags == NULL) {
        //
	//  Initialize imin and imax
        //
	imin = *result_imin;
	imax = *result_imax;

        //
        //  Only certain datatypes are supported.
        //
        int alloc_space;
	switch(datatype) {
	case XIL_BIT:       
          alloc_space = 2;
          break;
	case XIL_UNSIGNED_4:
          alloc_space = 16;
          break;
	case XIL_SIGNED_8:
	case XIL_BYTE:
          alloc_space = 256;
          break;
	case XIL_UNSIGNED_16:
	case XIL_SHORT:
          alloc_space = 65536;
          break;
	}

        flags = new Xil_unsigned8[alloc_space];
	if(flags == NULL) {
	    XilSystemState* state = (getSrcImage(1))->getSystemState();
	    XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", FALSE);
            mutex.unlock();
	    return XIL_FAILURE;
	}
        xili_memset(flags, 0, alloc_space);
    }

    //
    //  Update the local flags table
    //
    int start = *result_imin;
    int stop = *result_imax;

    //
    //  Special case as short values are negative
    //
    if(datatype == XIL_SHORT) {
	start += 32768;
	stop  += 32768;
    }
	
    for(i=start; i<=stop; i++) {
        //
	//  If there is a value in the input and not locally
	//  set the local value.
        //
	if(result_flags[i]) {
	    flags[i] = 1;
        }
    }

    if(*result_imin < imin) {
	imin = *result_imin;
    }

    if(*result_imax > imax) {
	imax = *result_imax;
    }

    mutex.unlock();
    return XIL_SUCCESS;
}

//
//  Build the final lut table to be returned to the API call
//
XilStatus
XilOpSqueezeRange::completeResults()
{
    int			i, k;
    unsigned int	table_size;
    int			new_index;
    XilSystemState*	state;

    //
    //  Initialization stage
    //
    table_size = imax - imin + 1;
    state = (getSrcImage(1))->getSystemState();

    if(datatype == XIL_SHORT) {
	//
	// Short needs an Xil_signed16 table
	//
        Xil_signed16* table_short = new Xil_signed16[table_size];
	if(table_short == NULL) {
	    XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-210", FALSE);
	    mutex.unlock();
	    return XIL_FAILURE;
	}
	
        //
        //  Just in case there was a failure...
        //
        if(flags == NULL) {
            table_short[0] = 0;
        } else {
            int start = imin + 32768;
            int stop = imax + 32768;

            new_index = imin - 1;
            for(i=start, k=0; i<=stop; i++, k++) {
                if(flags[i]) {
                    //
                    //  If pixel value represented, increment new index,
                    //  else use last value.
                    //
                    ++new_index;
                }
                table_short[k] = (Xil_signed16)new_index;
            }
        }

        //
	//  Build lookup to be rtn'd, get the System state from the
	//  source image first.
        //
	(*user_lut) = state->createXilLookupSingle(datatype, datatype, 1, 
						   table_size,
						   imin, table_short);

	// Check the lut before we return
	if((*user_lut) == NULL) {
	    XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-210", FALSE);
	    mutex.unlock();
	    return XIL_FAILURE;
	}

	// Now we can delete the table
	delete table_short;
    } else {
        Xil_unsigned8* table_byte = new Xil_unsigned8[table_size];
	if(table_byte == NULL) {
	    XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-210", FALSE);
	    mutex.unlock();
	    return XIL_FAILURE;
	}

        //
        //  Just in case there was a failure...
        //
        if(flags == NULL) {
            table_byte[0] = 0;
        } else {
            new_index = -1;
            for(i=imin, k=0; i<=imax; i++, k++) {
                if(flags[i]) {
                    //
                    //  If pixel value represented, increment new index,
                    //  else use last value.
                    //
                    ++new_index;
                }
                table_byte[k] = (Xil_unsigned8)new_index;
            }
        }

        //
	//  Build lookup to be rtn'd, get the System state from the
	//  source image first.
        //
	(*user_lut) = state->createXilLookupSingle(datatype, datatype, 1, 
						   table_size,
						   imin, table_byte);

        //
	//  Check the lut before we return
        //
	if((*user_lut) == NULL) {
	    XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-210", FALSE);
	    mutex.unlock();
	    return XIL_FAILURE;
	}

        //
	//  Now we can delete the table
        //
	delete table_byte;
    }

    //
    //  Clean up
    //
    delete flags;

    return XIL_SUCCESS;
}
