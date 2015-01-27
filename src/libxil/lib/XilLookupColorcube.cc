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
//  File:    XilLookupColorcube.cc
//  Project:    XIL
//  Revision:    1.24
//  Last Mod:    10:08:36, 03/10/00
//
//  Description:
//    Implementation of XilLookupColorcube class
//    This derives from the XilLookupSingle class
//    
//    
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilLookupColorcube.cc	1.24\t00/03/10  "
 
//
//  C++ Includes
//
#include "_XilDefines.h"
#include "_XilLookupColorcube.hh"
#include "_XilSystemState.hh"
#include "XiliUtils.hh"

struct DimMulPair {
    int          mult;
    unsigned int dim;
};

static int
compareFunction(const void* arg1, 
                const void* arg2)
{
    const DimMulPair* one = (const DimMulPair*)arg1;
    const DimMulPair* two = (const DimMulPair*)arg2;

    if(one->mult > two->mult) {
        return 1;
    }

    if(one->mult < two->mult) {
        return (-1);
    }

    return 0;
}

XilLookupColorcube::~XilLookupColorcube()
{
    if(colorcube_arrays != NULL)
        delete [] colorcube_arrays;
}

//
//  Copy constructor for color cube lookup.
//
XilLookupColorcube::XilLookupColorcube(XilSystemState*     system_state,
                                       XilLookupColorcube* orig_cube)
    : XilLookupSingle(system_state,
                      orig_cube->inputType, 
                      orig_cube->outputType, 
                      orig_cube->outputNBands, 
                      orig_cube->offset)
{
    isOKFlag = FALSE;

    //
    //  NULL this member so that destructor does not delete it when NULL
    //
    colorcube_arrays = NULL;

    //
    // Copy the contents of the original object to this object
    //
    this->entries         = orig_cube->entries;
    this->lookupType      = orig_cube->lookupType;
    this->inputNBands     = orig_cube->inputNBands;
    this->bytesPerEntry   = orig_cube->bytesPerEntry;
    this->bytesPerBand    = orig_cube->bytesPerBand;
    this->isColorcubeFlag = orig_cube->isColorcubeFlag;
    this->adjustedOffset  = orig_cube->adjustedOffset;

    //
    // Allocate and fill in the arrays with the contents of orig_cube.
    // These new pointers will replace the values of the old pointers.
    //
    colorcube_arrays = new int[3 * outputNBands];
    if(colorcube_arrays == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return;
    }

    multipliers = (int*)colorcube_arrays;
    dimensions = (unsigned int*)(colorcube_arrays + outputNBands);
    dimsMinus1 = (unsigned int*)(colorcube_arrays + 2*outputNBands);
    
    data = (void*)new char[entries*bytesPerEntry];
    if(data == NULL) {
        delete [] colorcube_arrays;
        XIL_ERROR(system_state,  XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    xili_memcpy(multipliers, orig_cube->multipliers, outputNBands*sizeof(int));
    xili_memcpy(dimensions, orig_cube->dimensions, outputNBands*sizeof(int));
    xili_memcpy(dimsMinus1, orig_cube->dimsMinus1, outputNBands*sizeof(int));
    xili_memcpy(data, orig_cube->data, entries*bytesPerEntry);

    isOKFlag = TRUE;
}

//
//  Constructor for color cube lookup.
//
XilLookupColorcube::XilLookupColorcube(XilSystemState* system_state,
                                       XilDataType     input_type,
                                       XilDataType     output_type,
                                       unsigned int    nbands,
                                       int             off,
                                       int*            mult,
                                       unsigned int*   dim) 
    : XilLookupSingle(system_state, input_type, output_type, nbands, off)
{
    unsigned int i;

    //
    //  NULL this member so that destructor does not delete it when NULL
    //
    colorcube_arrays = NULL;

    //
    //  Verify base classes constructed OK
    //
    if(! isOKFlag) {
        multipliers = NULL;
        dimensions  = NULL;
        dimsMinus1  = NULL;
        return;
    }

    isOKFlag = FALSE;

    lookupType = XIL_LOOKUP_COLORCUBE;

    multipliers = NULL;
    dimensions  = NULL;
    dimsMinus1  = NULL;

    if(mult == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-302", TRUE);
        return;
    }

    if(dim == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-252", TRUE);
        return;
    }

    //
    // Test if any of the dimensions or multipliers are zero
    //
    for(i=0; i<nbands; i++) {
	if(dim[i] == 0){
            XIL_ERROR(system_state, XIL_ERROR_USER, "di-252", TRUE);
            return;
        }
    }

    //
    //  How big is the color cube?
    //
    //  NOTE: Changed the calculation to use double precision float
    //        to avoid possibility of integer overflow
    //
    double fsize = 1.0;
    for(i=0; i<nbands; i++) {
        fsize *= (double) dim[i];
    }
    if(fsize > (double)0xffffffffU) {
        //
        //  Color cube is too large for 32 bit addressability
        //
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-129", TRUE);
        return;
    }
    int size = (int)fsize;

    //
    // Make sure the parameters don't go outside the maximum 
    //
    if((size + off) > (xili_get_datatype_max(input_type) + 1)) {
        //
        //  Lookup too large for input data type
        //
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-127", TRUE);
        return;
    }

    //
    //  Sanity checks on the parameters.
    //
    //  There should be a relationship between the dimensions and the
    //  multipliers.
    //

    //
    //  Check that each of the dimension/multiplier pairs result in a ramp
    //  that can fit in the table. 
    //
    for(i=0; i<nbands; i++) {
        if(dim[i] * _XILI_ABS(mult[i]) > (unsigned int)size) {
            XIL_ERROR(system_state, XIL_ERROR_USER, "di-128", TRUE);  
            return;
        }
    }

    //
    // Check that the multipliers are consistent with the dimensions
    // We need to sort them based on the absolute value of the multipliers. 
    // So put the mul/dim pairs in an array to be sorted by qsort.
    //
    DimMulPair* array = new DimMulPair[nbands];
    if(array == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    for(i=0; i<nbands; i++) {
        array[i].dim  = dim[i];
        array[i].mult = _XILI_ABS(mult[i]);
    }
    qsort(array, nbands, sizeof(DimMulPair), compareFunction);

    //
    //  With the array in sorted order, we can step through and test that the
    //  absolute value of each multiplier (after the first, which will
    //  always(?) be 1) is equal to the product of all the preceding
    //  multipliers. 
    //
    //  Gripe: This is a bizarre way to specify color cubes.
    //         It seems like it would be better to just specify
    //         the cube dimensions and a sign for each axis - to
    //         indicate if its a positive or negative ramp. Is
    //         there ever a need to specify something different
    //         than this ???  (lperry)
    //
    int step = 1;
    for(i=1; i<nbands; i++) {
        //
        //  Only test cube validity for dimensions > 1, since dim=1 means
        //  a constant value for a band, which won't effect cube validity.
        //
        if(array[i-1].dim > 1) {
            step *= array[i-1].dim;
            if(step != array[i].mult) {
                XIL_ERROR(system_state, XIL_ERROR_USER, "di-128", TRUE);  
                delete [] array;
                return;
            }
        }

    }
    delete [] array;

    //
    //  Set up the type of the lookup
    //
    inputNBands     = 1;
    outputNBands    = nbands;
    isColorcubeFlag = TRUE;

    //
    //  Go ahead and copy the arrays
    //
    colorcube_arrays = new int[3 * outputNBands];
    if(colorcube_arrays == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return;
    }

    multipliers = (int*)colorcube_arrays;
    dimensions = (unsigned int*)(colorcube_arrays + outputNBands);
    dimsMinus1 = (unsigned int*)(colorcube_arrays + 2*outputNBands);
    
    xili_memcpy(multipliers, mult, outputNBands*sizeof(int));
    xili_memcpy(dimensions, dim, outputNBands*sizeof(unsigned int));

    //
    //  Multipliers for dimension 1 bands are advertised as don't cares but
    //  the code requires a particular value.
    //
    //  While we're running through the bands, compute dimsMinus1.
    // 
    for(i=0; i <nbands; i++) {
       if(dim[i] == 1) {
           multipliers[i] = size;
       }

       dimsMinus1[i] = dimensions[i] - 1;
    }

    //
    //  Set up the extent of the lookup 
    //
    this->offset = off;
    entries      = size;

    //
    //  Set the adjusted offset.
    //
    adjustedOffset = off;
    for(i=0; i<outputNBands; i++) {
        if((dimensions[i] > 1) && (multipliers[i] < 0)) {
            adjustedOffset += _XILI_ABS(multipliers[i]) * dimsMinus1[i];
        }
    }

    //
    //  Maximum number of entries is 65536, the maximum number
    //  of bands per entry is 65535, and the maximum number of
    //  bytes per band is currently 2. Test for integer overflow 
    //  before computing the size of the lookup table.
    //
    bytesPerEntry = outputNBands * xili_sizeof(output_type);

    unsigned int max_bytes_per_entry = INT_MAX/entries;
    if(max_bytes_per_entry < bytesPerEntry) {
        //
        //  Lookup too large
        //
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-129", TRUE);
        return;
    }

    //
    //  Go ahead and compute the size of the lookup and allocate it 
    //
    data = (void*)new char[entries*bytesPerEntry];
    if(data == NULL) {
        XIL_ERROR(system_state,  XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    double output_min;
    double output_max;

    if(xili_is_floating_datatype(output_type)) {
        //
        //  Floats are assumed normalized 0.0 to 1.0
        //
        output_min = 0.0;
        output_max = 1.0;
    } else {
        output_min = xili_get_datatype_min(output_type);
        output_max = xili_get_datatype_max(output_type);
    }

    //
    //  Populate each band of the lookup table.
    //
    //  We use the multiplier to determine how many repeats of a value are
    //  needed before going to the next.  The dimension determines the number
    //  of values. 
    //
    double delta;
    double start;
    double val;
    unsigned int k;

    for(unsigned int band=0; band<outputNBands; band++) {
        //
        // A dimension of one means all entries will be the same
        //
        if(dimensions[band] == 1){
            delta = 0.0;
        }else {
            if (xili_is_floating_datatype(output_type)) {
                delta = 1.0 / (dimensions[band] - 1);
            } else {
                delta = (output_max - output_min) / (dimensions[band] - 1);
            }
        }

        //
        // A negative multiplier means that the ramp 
        // for that band will be reversed (high to low),
        // So we need to start at the max and add a negative delta.
        //
        if(multipliers[band] < 0) {
            delta = -delta;
            start = output_max;
        } else {
            start = output_min;
        }
        int repeat_count = _XILI_ABS(multipliers[band]);

        switch(output_type) {
          case XIL_BIT:
          case XIL_UNSIGNED_4:
          case XIL_BYTE:
            k = band;
            while(k<size*nbands) {
                val = start;
                for(i=0; i<dimensions[band]; i++) {
                    for(int j=0; j<repeat_count; j++) {
                        *((Xil_unsigned8*)data + k) =(Xil_unsigned8)(val+0.5);
                        k += nbands;
                    }
                    val += delta;
                }
            }
            break;

          case XIL_SIGNED_8:
            k = band;
            while(k<size*nbands) {
                val = start;
                for(i=0; i<dimensions[band]; i++) {
                    for(int j=0; j<repeat_count; j++) {
                        *((Xil_signed8*)data + k) = (Xil_signed8)(val + 0.5);
                        k += nbands;
                    }
                    val += delta;
                }
            }
            break;

          case XIL_SHORT:
            k = band;
            while(k<size*nbands) {
                val = start;
                for(i=0; i<dimensions[band]; i++) {
                    for(int j=0; j<repeat_count; j++) {
                        *((Xil_signed16*)data + k) =(Xil_signed16)(val + 0.5);
                        k += nbands;
                    }
                    val += delta;
                }
            }
            break;

          case XIL_UNSIGNED_16:
            k = band;
            while(k<size*nbands) {
                val = start;
                for(i=0; i<dimensions[band]; i++) {
                    for(int j=0; j<repeat_count; j++) {
                       *((Xil_unsigned16*)data + k)=(Xil_unsigned16)(val+0.5);
                       k += nbands;
                    }
                    val += delta;
                }
            }
            break;

          case XIL_SIGNED_32:
            k = band;
            while(k<size*nbands) {
                val = start;
                for(i=0; i<dimensions[band]; i++) {
                    for(int j=0; j<repeat_count; j++) {
                        *((Xil_signed32*)data + k) =(Xil_signed32)(val+0.5);
                        k += (int)nbands;
                    }
                    val += delta;
                }
            }
            break;

          case XIL_UNSIGNED_32:
            k = band;
            while(k<size*nbands) {
                val = start;
                for(i=0; i<dimensions[band]; i++) {
                    for(int j=0; j<repeat_count; j++) {
                      *((Xil_unsigned32*)data + k) =(Xil_unsigned32)(val+0.5);
                      k += (int)nbands;
                    }
                    val += delta;
                }
            }
            break;

          case XIL_FLOAT:
            k = band;
            while(k<size*nbands) {
                val = start;
                for(i=0; i<dimensions[band]; i++) {
                    for(int j=0; j<repeat_count; j++) {
                        *((Xil_float32*)data + k) = (Xil_float32)val;
                        k += (int)nbands;
                    }
                    val += delta;
                }
            }
            break;

          case XIL_FLOAT_64:
            k = band;
            while(k<size*nbands) {
                val = start;
                for(i=0; i<dimensions[band]; i++) {
                    for(int j=0; j<repeat_count; j++) {
                        *((Xil_float64*)data + k) = (Xil_float64)val;
                        k += nbands;
                    }
                    val += delta;
                }
            }
            break;

          default:
            // Unsupported output data type - UNREACHABLE
            XIL_ERROR(system_state, XIL_ERROR_USER, "di-146", TRUE);
            break;
        }

    }

    isOKFlag = TRUE;
}

//
//  Get the colorcube information.
//
const int*
XilLookupColorcube::getMultipliers()
{
    return multipliers;
}

const unsigned int*
XilLookupColorcube::getDimensions()
{
    return dimensions;
}

const unsigned int*
XilLookupColorcube::getDimsMinus1()
{
    return dimsMinus1;
}

int
XilLookupColorcube::getAdjustedOffset()
{
    return adjustedOffset;
}

//
// Get a description of the color cube 
//
Xil_boolean 
XilLookupColorcube::getColorcubeInfo(int*          mults,
                                     unsigned int* dims,
                                     short*        adjusted_offset)
{
    if(!isColorcubeFlag) {
        return FALSE;
    } else {
        if(mults) {
            xili_memcpy(mults, multipliers, outputNBands*sizeof(int));
        }

        if(dims) {
            xili_memcpy(dims, dimensions, outputNBands*sizeof(unsigned int));
        }

        if(adjusted_offset) {
            *adjusted_offset = (short)adjustedOffset;
        }

        return TRUE;
    }
}

//
// Create a complete copy of the lookup 
//
XilObject* 
XilLookupColorcube::createCopy()
{
    XilLookupColorcube* new_lut = 
        getSystemState()->createXilLookupColorcube(this);

    if(!new_lut->isOK()) {
        XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-177", FALSE);
        return NULL;
    }

    //
    //  Give the copy the same version number
    //
    new_lut->copyVersionInfo(this);

    return new_lut;
}


//
// Change the offset of the lookup, the offset can't be set to greater than
// maxSize - numentries where maxSize is the maximum size for the input type
//
void
XilLookupColorcube::vSetOffset(int off)
{
    //
    // Update the parts of the object common to LookupSingle
    //
    XilLookupSingle::vSetOffset(off);

    //
    //  Also set the adjusted offset, which is unique to colorcubes.
    //
    adjustedOffset = off;
    for(unsigned int i=0; i<outputNBands; i++) {
        if((dimensions[i] > 1) && (multipliers[i] < 0)) {
            adjustedOffset += _XILI_ABS(multipliers[i]) * dimsMinus1[i];
        }
    }

}

