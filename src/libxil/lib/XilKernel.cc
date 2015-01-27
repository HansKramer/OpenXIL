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
//  File:    XilKernel.cc
//  Project:    XIL
//  Revision:    1.24
//  Last Mod:    10:08:40, 03/10/00
//
//  Description:
//    Implementation of XilKernel class
//    
//    
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilKernel.cc	1.24\t00/03/10  "
 
//
//  System Includes
//
#include <memory.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

//
//  C++ Inclues
//
#include "_XilDefines.h"
#include "_XilKernel.hh"
#include "_XilSystemState.hh"
#include "XiliUtils.hh"

XilKernel::~XilKernel()
{
    delete [] data;
    delete [] xData;
    delete [] yData;
}

//
//  Constructors
//
XilKernel::XilKernel(XilSystemState* system_state,
                     unsigned int    xsize,
                     unsigned int    ysize,
		     int	     key_x,
		     int             key_y,
		     float*          input_data)
: XilNonDeferrableObject(system_state, XIL_KERNEL)
{
    isOKFlag = FALSE;

    //
    //  Initialize to NULL just in case we fail.
    //
    data  = NULL;
    xData = NULL;
    yData = NULL;

    if(input_data == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return;
    }

    if((xsize==0) || (ysize==0)) {
       XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
       return;
    }

    if(((unsigned int)key_x >= xsize) || ((unsigned int)key_y >= ysize)) {
       XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
       return;
    }

    if((INT_MAX/sizeof(float)/xsize) < ysize) {
       XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-129", TRUE);
       return;
    }
 
    //
    //  Check the kernel for invalid values.
    //
    for(unsigned int i=0; i<(xsize*ysize); i++) {
        if(input_data[i] != input_data[i]) {
            XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-263", TRUE);
            return;
       } else if((input_data[i] == HUGE_VAL) || (input_data[i] == -HUGE_VAL)) {
           XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-264", TRUE);
           return;
       }
    }

    data = new float[xsize*ysize];
    if(data == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }
    xili_memcpy(data, input_data, sizeof(float)*xsize*ysize);

    width  = xsize;
    height = ysize;
    keyX   = key_x;
    keyY   = key_y;

    //
    //  kernel doesn't have to be square but
    //  key values should be centered
    //  If it's a one-dimensional kernel, this is moot
    //
    if((unsigned int)keyX == xsize>>1 &&
       (unsigned int)keyY == ysize>>1 &&
        (width != 1) &&
        (height != 1) ){
        checkSeparable();
    }

    //
    //  AFTER THIS POINT AN ERROR MUST delete data and set it to NULL 
    //

    isOKFlag = TRUE;
}


//
//  Constructors
//
XilKernel::XilKernel(XilSystemState* system_state,
                     unsigned int    xsize,
                     unsigned int    ysize,
		     int	     key_x,
		     int             key_y,
		     float*          x_data,
                     float*          y_data)
: XilNonDeferrableObject(system_state, XIL_KERNEL)
{
    isOKFlag = FALSE;

    //
    //  Initialize to NULL just in case we fail.
    //
    data  = NULL;
    xData = NULL;
    yData = NULL;


    if(x_data == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return;
    }

    if(y_data == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return;
    }

    if((xsize==0) || (ysize==0)) {
       XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
       return;
    }

    if(((unsigned)key_x >= xsize) || ((unsigned)key_y >= ysize)) {
       XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
       return;
    }

    //
    //  Check the kernel for invalid values.
    //
    unsigned int i;
    for(i=0; i<xsize; i++) {
        if(x_data[i] != x_data[i]) {
            XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-263", TRUE);
            return;
       } else if((x_data[i] == HUGE_VAL) || (x_data[i] == -HUGE_VAL)) {
           XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-264", TRUE);
           return;
       }
    }

    for(i=0; i<ysize; i++) {
        if(y_data[i] != y_data[i]) {
            XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-263", TRUE);
            return;
       } else if((y_data[i] == HUGE_VAL) || (y_data[i] == -HUGE_VAL)) {
           XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-264", TRUE);
           return;
       }
    }

    xData = new float[xsize];
    if(xData == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }
    xili_memcpy(xData, x_data, sizeof(float)*xsize);

    yData = new float[ysize];
    if(yData == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }
    xili_memcpy(yData, y_data, sizeof(float)*ysize);

    width  = xsize;
    height = ysize;
    keyX   = key_x;
    keyY   = key_y;

    //
    // AFTER THIS POINT AN ERROR MUST delete data and set it to NULL 
    //

    isOKFlag = TRUE;
}

//
//  checkSeparable() is called by the non-separable constructor to see if the
//  kernel data is separable.  It's only to be called if xsize == ysize and
//  the key is at the center.
//
void
XilKernel::checkSeparable()
{
    if(data[0] == 0.0F) {
        return;
    }

    //
    //  We don't worry about generating errors here because the kernel has
    //  already successfully be constructed and will work, this is just an
    //  optimization that's failing.  tmp_data is the first column (vector)
    //  of the kernel.
    //
    unsigned int i, j;
    float* tmp_data = new float[height];
    if(tmp_data == NULL) {
        return;
    }

    //
    //  Divide the first column by the first element.
    //
    float fac = 1.0F / data[0];
    tmp_data[0] = 1.0F;

    for(i=1; i<height; i++) {
        tmp_data[i] = data[i*width]*fac;
    }

    //
    //  Check to see if things multiply out to match the kernel.
    //
    for( i = 0; i < height; i++ ) {
        for( j = 0; j < width; j++ ) {
           if ( ! XILI_FLT_EQ((data[j]*tmp_data[i]), data[i*width+j]) ) {
              delete[] tmp_data;
              return;
           }
        }
    }

    xData = new float[width];
    if(xData == NULL) {
        delete tmp_data;
        return;
    }
    xili_memcpy(xData, data, width*sizeof(float));

    yData = tmp_data;
}

//
//  MT-safe Locking required in this object to protect the "data" pointer
//  because it gets created on-the-fly by invert() and getData().  Otherwise,
//  no other object information changes after construction.
//

//
//  Test whether this kernel is separable or not.
//
Xil_boolean
XilKernel::isSeparable()
{
    if(xData != NULL && yData != NULL) {
        return TRUE;
    } else {
        return FALSE;
    }
}

//
// Create a copy of the kernel
//
XilObject*
XilKernel::createCopy()
{
    XilKernel* new_copy = NULL;

    if(xData != NULL && yData != NULL) {
        //
        //  Create new separable kernel...
        //
        new_copy =
            getSystemState()->createXilKernel(width, height, keyX, keyY,
                                              xData, yData);
    } else {
        //
        //  Create regular kernel...
        //
        new_copy =
            getSystemState()->createXilKernel(width, height, keyX, keyY, data);
    }
    
    if(new_copy == NULL) {
	XIL_ERROR(this->getSystemState(), XIL_ERROR_SYSTEM, "di-163", FALSE);
	return NULL;
    }

    new_copy->copyVersionInfo(this);

    return new_copy;
}

unsigned int 
XilKernel::getWidth()
{
    return width;
}

unsigned int 
XilKernel::getHeight()
{
    return height;
}

int 
XilKernel::getKeyX()
{
    return keyX;
}

int 
XilKernel::getKeyY()
{
    return keyY;
}

void
XilKernel::getValues(float* data_arg)
{
    if(data_arg == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-259", TRUE);
        return;
    }

    // 
    // Get the ptr to the kernel data
    // This will combine separable kernels if needed
    //
    const float* src_ptr = getData();

    //
    // Copy to the user-supplied buffer
    //
    if(src_ptr != NULL) {
        xili_memcpy(data_arg, src_ptr, width*height*sizeof(float));
    }

}

const float*
XilKernel::getData()
{
    kernelMutex.lock();

    if(data == NULL && isSeparable()) {
        //
        // Populate the 2-D array by multiplying the two separable
        // 1-D kernel vectors together (as a cross-product)
        //
        data = new float[width*height];
        if(data == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            kernelMutex.unlock();
            return NULL;
        }

        float* tmp_ptr = data;
        for(unsigned int y=0; y<height; y++) {
            for(unsigned int x=0; x<width; x++) {
                *tmp_ptr++ = xData[x] * yData[y];
            }
        }
    }

    kernelMutex.unlock();

    return data;
}

//
//  Support for special-case separable kernels.
//
void
XilKernel::getSeparableData(const float** x_array,
                            const float** y_array)
{
    *x_array = xData;
    *y_array = yData;
}

void
XilKernel::invert()
{
    //
    //  We're modifying the kernel so update the version number prior to
    //  modifying the information contained in it.
    //
    newVersion();

    //
    //  Internal function, don't need to test for NULL.
    //
    kernelMutex.lock();

    //
    //  Check to see if we invert the separable arrays or the full array.
    //
    if(data == NULL && xData != NULL && yData != NULL) {
        //
        //  Invert the separable data arrays.
        //
        float* tmpx = new float[width];
        if(tmpx == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            kernelMutex.unlock();
            return;
        }

        int i;
        int j;
        for(i=0, j=width-1; j>=0; i++, j--) {
            tmpx[j] = xData[i];
        }

        delete [] xData;

        xData = tmpx;

        float* tmpy = new float[height];
        if(tmpy == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            kernelMutex.unlock();
            return;
        }

        for(i=0, j=height-1; j>=0; i++, j--) {
            tmpy[j] = yData[i];
        }

        delete [] yData;

        yData = tmpy;
    } else {
        int i;
        float* temp = new float[width*height];
        if(temp == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            kernelMutex.unlock();
            return;
        }
        
        for(i=0; i<width*height; i++) {
            temp[i] = data[i];
        }
    
        float *temp_save = temp;
        for(i=width*height - 1; i>=0; i--) {
            data[i] = *temp++;
        }

        delete [] temp_save;
    }

    //
    //  We've inverted the data now invert the key
    //  values so we are still centered correctly.
    //
    keyX = width  - keyX - 1;
    keyY = height - keyY - 1;

    kernelMutex.unlock();
}
