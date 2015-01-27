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
//  File:       XilOpChooseColormap.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:07:52, 03/10/00
//
//  Description:
//
//    Op for ChooseColormap function.
//    This is a combination of a specialized histogram,
//    which is implemented in the ChooseColormap compute routine,
//    and a histogram analysis method which is implemented
//    in the completeResults method of this op.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilOpChooseColormap.cc	1.6\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilOpDataCollect.hh"

class XilOpChooseColormap : public XilOpDataCollect {
public:
    static XilOp*       create(char* function_name,
                               void* args[],
                               int count);

protected:
    // Constructor takes in the datatype of the image and
    // the pointer to return the created lookup table
    XilOpChooseColormap(XilOpNumber  op_num, 
                        unsigned int num_entries,
                        XilLookup**  lut);

    virtual ~XilOpChooseColormap();


private:
    // Lookup object to be returned to the user
    XilLookup**         user_lut;

    unsigned int        cmap_size;

};

XilOp*
XilOpChooseColormap::create(char  function_name[], 
                            void* args[], 
                            int)
{
    //
    //  There is no dst image, but xili_verify_op_args expects there
    //  to be one, 1.2 did it this way
    //
    XilImage*           src         = (XilImage*)args[0];
    unsigned int        num_entries = *((unsigned int*)args[1]);

    //
    // This is the address of a pointer to an XilLookup object.
    // The pointer is supplied by the c_bindings, but the
    // allocation of the XilLookup object is done in this Op.
    // Once the XilLookup object is created and populated
    // with the new cmap, its address is copied into the
    // pointer at this location.
    //
    XilLookup**         lut         = (XilLookup**)args[2];

    static XilOpCache        choose_colormap_op_cache;
    XilOpNumber                opnum;
    if((opnum = xili_verify_op_args(function_name, &choose_colormap_op_cache,
                                    src, src)) == -1) {
        return NULL;
    }

    //
    //  Only works for 3-banded images
    //
    if(src->getNumBands() != 3) {
        XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-167", TRUE);
        return NULL;
    }

    //
    // This operation only works for BYTE images
    //
    if(src->getDataType() != XIL_BYTE) {
        // Operation not implemented for other than BYTE images
        XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-5", TRUE);
        return NULL;
    }

    //
    //  Now create the op and set the op args
    //
    XilOpChooseColormap* op = new XilOpChooseColormap(opnum, num_entries, lut);
    if(op == NULL) {
        XIL_ERROR(src->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    op->setSrc(1, src);
    op->setParam(1, num_entries);
    op->setParam(2, (void*)lut, XIL_DONT_DELETE);

    return op;
}

// Constructor
XilOpChooseColormap::XilOpChooseColormap(XilOpNumber  op_num,
                                         unsigned int num_entries,
                                         XilLookup**  lut)
 : XilOpDataCollect(op_num)
{
    //
    // Store the  requested colormap size and
    // the address of the XilLookup handle.
    //
    cmap_size  = num_entries;
    user_lut   = lut;
}

// Destructor
XilOpChooseColormap::~XilOpChooseColormap() 
{ 
}
