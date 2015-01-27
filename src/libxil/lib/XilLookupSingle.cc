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
//  File:       XilLookupSingle.cc
//  Project:    XIL
//  Revision:   1.26
//  Last Mod:   10:08:56, 03/10/00
//
//  Description:
//
//    Implementation of XilLookupSingle class
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilLookupSingle.cc	1.26\t00/03/10  "
 
#include <string.h>

//
//  C++ Includes
//
#include "_XilDefines.h"
#include "_XilLookupSingle.hh"
#include "_XilSystemState.hh"
#include "XiliUtils.hh"

XilLookupSingle::~XilLookupSingle()
{
    if(data != NULL)
        delete [] data;
}

//
//  Constructor for an arbitrary lookup.
//
XilLookupSingle::XilLookupSingle(XilSystemState* system_state,
                                 XilDataType     input_type,
                                 XilDataType     output_type,
                                 unsigned int    nbands,
                                 unsigned int    count,
                                 int             off,
                                 void*           lutdata)
: XilLookup(system_state, input_type, output_type)
{
    //
    //  NULL this member so that destructor does not delete it when NULL
    //
    data = NULL;

    //
    // Verify base class OK
    //
    if(! isOKFlag) {
        return;
    }

    isOKFlag = FALSE;

    lookupType = XIL_LOOKUP_SINGLE;
    data       = NULL;

    if(nbands == 0) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-243", TRUE);
        return;
    }

    if(count == 0) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-166", TRUE);
        return;
    }

    if((off < 0) && ! xili_is_signed_integer_datatype(input_type)) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-146", TRUE);
        return;
    }

    //
    //  Make sure the parameters don't go outside the maximum
    //
    if(((int)count+off) > (int)maxSize) {
        //
        //  Lookup too large for input data type.
        //
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-127", TRUE);
        return;
    }

    inputNBands  = 1;
    outputNBands = nbands;

    //
    //  Set up the extent of the lookup
    //
    offset       = off;
    entries      = count;

    //
    // Test for integer overflow of allocation amount
    // 
    bytesPerEntry = nbands*xili_sizeof(output_type);

    unsigned int max_bytes_per_entry = INT_MAX/entries;
    if(max_bytes_per_entry < bytesPerEntry) {
        //
        //  Lookup too large to be addressed
        //
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-129", TRUE);
        this->data = NULL;
        return;
    }

    //
    //  Go ahead and compute the size of the lookup and allocate it
    //
    int size   = (int)(entries*bytesPerEntry);
    this->data = (void*)new char [size];
    if(this->data == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    //
    //  Copy the data or clear to 0
    //
    if(lutdata == NULL) {
        memset(this->data, 0, size);
    } else {
        memcpy(this->data, lutdata, size);
    }

    isOKFlag = TRUE;
}

//
//  Constructor for an arbitrary lookup.
//
XilLookupSingle::XilLookupSingle(XilSystemState* system_state,
                     XilDataType     input_type,
                     XilDataType     output_type,
                     unsigned int    nbands,
                     int             off)
: XilLookup(system_state, input_type, output_type)
{
    //
    //  NULL this member so that destructor does not delete it when NULL
    //
    data = NULL;

    //
    // Verify base class OK
    //
    if(! isOKFlag) {
        return;
    }

    isOKFlag = FALSE;

    lookupType = XIL_LOOKUP_SINGLE;
    data       = NULL;

    if(nbands == 0) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-243", TRUE);
        return;
    }

    if((off < 0) && ! xili_is_signed_integer_datatype(input_type)) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-146", TRUE);
        return;
    }

    inputNBands  = 1;
    outputNBands = nbands;

    //
    //  Set up the extent of the lookup
    //
    offset       = off;
    bytesPerEntry = nbands * xili_sizeof(output_type);

    isOKFlag = TRUE;
}


//
// Create a complete copy of the lookup 
//
XilObject* 
XilLookupSingle::createCopy()
{
    XilLookupSingle* new_lut =
        getSystemState()->createXilLookupSingle(inputType, outputType,
                                                outputNBands, entries,
                                                offset, data);
    if(new_lut == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-177", FALSE);
        return NULL;
    }

    //
    // Give the copy the same version number
    //
    new_lut->copyVersionInfo(this);

    return new_lut;
}


unsigned int    
XilLookupSingle::getNumEntries()
{
    return entries;
}

const void*         
XilLookupSingle::getData()
{ 
    return data; 
}

int        
XilLookupSingle::getOffset()
{
    return offset;
}

//
// Change the offset of the lookup, the offset can't be set to greater than  
// maxSize - numentries where maxSize is the maximum size for the input type 
//
void 
XilLookupSingle::setOffset(int off)
{
    //
    // Call the virtual function. If this lookup is a colorcube,
    // the adjusted offset is also set
    //
    vSetOffset(off);
}

void 
XilLookupSingle::vSetOffset(int off)
{
    //
    //  Make sure the offset isn't too large
    //
    if(((int)entries+off) > (int)maxSize) {
        //
        //  Lookup too large for input data type
        //
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-127", TRUE, this);
        return;   
    }

    //
    //  Update the version number (prior to changing the object)
    //
    newVersion();

    this->offset = off;
}

//
// Set the value of some entries, this will return an error if 
// the the indexes go outside of the side of the lookup        
//
void 
XilLookupSingle::setValues(int          start, 
                           unsigned int count,
                           const void*  lutdata)
{
    if(lutdata == NULL) {
      XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
      return;
    }

    //
    //  Range check needs to be done with ints to prevent overflow
    //
    if((start < offset) ||
       (((int) (start+count)) > ((int) (offset+entries)))) {
        //
        //  Data goes outside the range of this lookup
        //
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-130", TRUE, this);
        return;
    }

    //
    //  Update the version number (prior to changing the object)
    //
    newVersion();

    //
    //  Copy the data
    //
    unsigned char* dstaddr = (unsigned char*)this->data;
    dstaddr = dstaddr+ (start-offset)*bytesPerEntry;

    //
    //  The overall size is guaranteed to fit into an int
    //
    memcpy(dstaddr, lutdata, (int)(count*bytesPerEntry));

    //
    //  We'll assume that if the user created it as a colorcube, if they
    //  change the values, they changed them to colorcube values.
    //
}

//
// Get the value of some entries, this will return an error if  
// the the indexes go outside of the side of the lookup, or if  
// it cannot allocate the buffer used to return the data values 
//
void 
XilLookupSingle::getValues(int          start, 
                           unsigned int count, 
                           void*        buffer)
{
    if(buffer == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return;
    }


    //
    //  Range check needs to be done with ints to prevent overflow
    //
    if((start < offset) ||
       ((start+count) > (offset+entries))) {
        //
        //  Data goes outside the range of this lookup
        //
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-130", TRUE, this);
        return;
    }

    //
    //  Copy the data
    //
    unsigned char* srcaddr = (unsigned char*)this->data;
    srcaddr = srcaddr + (start-offset)*bytesPerEntry;

    //
    //  We assume the user-supplied buffer is big enough
    //
    xili_memcpy(buffer, srcaddr, (int)(count*bytesPerEntry));
}

//
// Implementation of lookup->convert 
// NOTE: This function will also take a colorcube as input
//       since colorcube inherits from LookupSingle.
//       However, the resulting lookup type is Single, since the
//       values of the multipliers and dimensions will 
//       almost certainly change.
//
XilLookup* 
XilLookupSingle::convert(XilLookup* dst_lut)
{
    XilLookupSingle* single_dst_lut = (XilLookupSingle*)dst_lut;

    if(!dst_lut) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-131", TRUE,
                      (XilObject*)this);
        return NULL;
    }

    //
    // Both LUTs must have same input data type
    //
    if(inputType != dst_lut->getInputDataType()) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-132", TRUE,
                      (XilObject*)this);
        return NULL;
    }

    //
    // Both LUTs must have same output data type
    //
    if(outputType != dst_lut->getOutputDataType()) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-132", TRUE,
                      (XilObject*)this);
        return NULL;
    }

    //
    // Both LUTs must have same number of bands
    //
    if(outputNBands !=  dst_lut->getOutputNBands()) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-132", TRUE,
                      (XilObject*)this);
        return NULL;
    }

    //
    // Limit the operation to SHORT lookups or smaller
    //
    if(xili_sizeof(inputType)                   > sizeof(short) ||
       xili_sizeof(dst_lut->getInputDataType()) > sizeof(short))
    {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-132", TRUE,
                      (XilObject*)this);
        return NULL;
    }

    //
    // BytesPerBand of conversion LUT is size of lut1 input type
    //
    unsigned int   cvt_elem_size    = xili_sizeof(inputType);

    Xil_unsigned8* src_data         = (Xil_unsigned8*)this->getData();
    Xil_unsigned8* dst_data         = (Xil_unsigned8*)single_dst_lut->getData();
    unsigned int   src_entries      = entries;
    unsigned int   dst_entries      = single_dst_lut->getNumEntries();

    //
    // Allocate space for conversion LUT
    // Make sure we get 16 bit alignment.
    // The conversion LUT is single-banded.
    //
    unsigned int cvt_lut_size = src_entries*cvt_elem_size+1;
    Xil_unsigned8* cvt_data = (Xil_unsigned8*)new Xil_signed16[cvt_lut_size/2];

    int src_offset          = offset;
    int dst_offset          = single_dst_lut->getOffset();

    Xil_unsigned8* pcvt     = cvt_data;
    Xil_unsigned8* lut1     = src_data;
    Xil_unsigned8* lut2;

    //
    // For each element of lut 1, do an exhaustive search of lut2
    // to find the closest match as measured by N band euclidean distance
    //
    double closest_dist;
    double distance;
    for(unsigned int isrc=0; isrc<src_entries; isrc++) {

        lut2 = dst_data;
        closest_dist = euclideanDistanceSquare(lut1, lut2);
        int closest_entry = 0;

        lut2 += bytesPerEntry;;

        for(unsigned int idst=1; idst<dst_entries; idst++) {
            distance = euclideanDistanceSquare(lut1, lut2);
            if(distance < closest_dist) {
                closest_dist  = distance;
                closest_entry = idst;
            }
            lut2 += bytesPerEntry;;  
        }

        //
        // Adjust for the lut offset
        //
        closest_entry += dst_offset;

        //
        // Assign the entry in conversion LUT
        //
        switch(inputType) {
          case XIL_BIT:    
          case XIL_BYTE:    
          case XIL_UNSIGNED_4:
            *((Xil_unsigned8*)pcvt)  = closest_entry;
            break;
          case XIL_SIGNED_8:
            *((Xil_signed8*)pcvt)    = closest_entry;
            break;
          case XIL_SHORT:    
            *((Xil_signed16*)pcvt)   = closest_entry;
            break;
          case XIL_UNSIGNED_16:    
            *((Xil_unsigned16*)pcvt) = closest_entry;
            break;
          default:    // Invalid output type - UNREACHABLE 
            XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-157", TRUE);
        }

        lut1 += bytesPerEntry;;
        pcvt += cvt_elem_size;

    }

    //
    // Create conversion LUT
    //
    XilLookupSingle* cvt_LUT = new XilLookupSingle(this->getSystemState(), 
                                           inputType, inputType, 
                                           (unsigned int)1, 
                                           src_entries, src_offset, 
                                           cvt_data);
    delete [] cvt_data;

    if(cvt_LUT == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-192", FALSE);
        return NULL;
    }

    if(cvt_LUT->data == NULL) {
        cvt_LUT->destroy();
        XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-192", FALSE);
        return NULL;
    }

    return cvt_LUT;
}

//
//  This routine calculates the square of Euclidian distance between
//  two color vectors. Overload for BYTE and SHORT types
//
double
XilLookupSingle::euclideanDistanceSquare(Xil_unsigned8* p0,
                                         Xil_unsigned8* p1)
{
    unsigned int i;
    int diff;
    double sum = 0;
 
    switch(outputType) {
      case XIL_BIT:    
      case XIL_BYTE:    
      case XIL_UNSIGNED_4:
        for(i=0; i<outputNBands; i++) {
            diff = *((Xil_unsigned8*)p0) - *((Xil_unsigned8*)p1);
            sum += (double)(diff * diff);
            p0 += bytesPerBand;
            p1 += bytesPerBand;
        }
        break;
      case XIL_SIGNED_8:
        for(i=0; i<outputNBands; i++) {
            diff = *((Xil_signed8*)p0) - *((Xil_signed8*)p1);
            sum += (double)(diff * diff);
            p0 += bytesPerBand;
            p1 += bytesPerBand;
        }
        break;
      case XIL_SHORT:    
        for(i=0; i<outputNBands; i++) {
            diff = *((Xil_signed16*)p0) - *((Xil_signed16*)p1);
            sum += (double)(diff * diff);
            p0 += bytesPerBand;
            p1 += bytesPerBand;
        }
        break;
      case XIL_UNSIGNED_16:    
        for(i=0; i<outputNBands; i++) {
            diff = *((Xil_unsigned16*)p0) - *((Xil_unsigned16*)p1);
            sum += (double)(diff * diff);
            p0 += bytesPerBand;
            p1 += bytesPerBand;
        }
        break;
      default: // Can't happen
        break;
    }
 
    return sum;
}

