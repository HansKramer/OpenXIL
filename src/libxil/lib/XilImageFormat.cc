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
//  File:	XilImageFormat.cc
//  Project:	XIL
//  Revision:	1.33
//  Last Mod:	10:08:07, 03/10/00
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
#pragma ident	"@(#)XilImageFormat.cc	1.33\t00/03/10  "

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilImageFormat.hh"
#include "_XilColorspace.hh"
#include "XiliUtils.hh"

XilImageFormat::XilImageFormat(XilSystemState* system_state,
                               unsigned int    x_size, 
                               unsigned int    y_size, 
                               unsigned int    n_bands,
                               XilDataType     datatype,
                               XilObjectType   object_type)
: XilDeferrableObject(system_state, object_type)
{
    isOKFlag   = FALSE;

    if(xili_is_supported_datatype(datatype) == FALSE) {
        XIL_OBJ_ERROR(system_state, XIL_ERROR_USER,
                      "di-134", TRUE, this);
        return;
    }
    
    xSize      = x_size;
    ySize      = y_size;
    xPixelSize = -1.0;
    yPixelSize = -1.0;
    nBands     = n_bands;
    colorspace = NULL;
    dataType   = datatype;
    
    isOKFlag   = TRUE;
}

XilImageFormat::XilImageFormat(XilSystemState* system_state,
                               XilImageFormat* image_format,
                               XilObjectType   object_type)
: XilDeferrableObject(system_state, object_type)
{
    isOKFlag = FALSE;
    
    image_format->getInfo(&xSize, &ySize, &nBands, &dataType);
    colorspace = image_format->getColorspace();
    
    isOKFlag = TRUE;
}

XilImageFormat::XilImageFormat(XilSystemState* system_state,
                               XilObjectType   object_type)
: XilDeferrableObject(system_state, object_type)
{
    isOKFlag = FALSE;
    
    colorspace = NULL;
    xSize      = 0;
    ySize      = 0;
    xPixelSize = -1.0;
    yPixelSize = -1.0;
    nBands     = 0;
    dataType   = XIL_BIT;
    
    isOKFlag = TRUE;
}

XilImageFormat::~XilImageFormat()
{
    //
    //  Things wern't initialized properly so return immediately.
    //
    if(isOKFlag == FALSE) {
        return;
    }

    colorspace->destroy();
}

//
// Create a copy of the image format
//
XilObject*
XilImageFormat::createCopy()
{
    XilImageFormat* new_copy =
        getSystemState()->createXilImageFormat(xSize, ySize, nBands, dataType);
    if(new_copy == NULL) {
 	XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM,
                      "di-440", TRUE, this);
	return NULL;
    }

    new_copy->copyVersionInfo(this);

    return new_copy;
}

unsigned int
XilImageFormat::getWidth()
{
    return xSize;
}

unsigned int
XilImageFormat::getHeight()
{
    return ySize;
}

float 
XilImageFormat::getPixelWidth()
{
    return xPixelSize;
}

float 
XilImageFormat::getPixelHeight()
{
    return yPixelSize;
}

void 
XilImageFormat::setPixelWidth(float width)
{
    xPixelSize = width;
}

void 
XilImageFormat::setPixelHeight(float height)
{
    yPixelSize = height;
}

unsigned int
XilImageFormat::getNumBands()
{
    return nBands;
}

XilDataType
XilImageFormat::getDataType()
{
    return dataType;
}

void
XilImageFormat::getSize(unsigned int* width,
                        unsigned int* height)
{
    if(width)  *width  = xSize;
    if(height) *height = ySize;
}

void
XilImageFormat::getInfo(unsigned int* width,
                        unsigned int* height,
                        unsigned int* nbands,
                        XilDataType*  datatype)
{
    if(width)    *width    = xSize;
    if(height)   *height   = ySize;
    if(nbands)   *nbands   = nBands;
    if(datatype) *datatype = dataType;
}

XilColorspace*
XilImageFormat::getColorspace()
{
    if(colorspace) {
	return (XilColorspace*)colorspace->createCopy();
    } else {
	return NULL;
    }
}

XilColorspace*
XilImageFormat::refColorspace()
{
    return colorspace;
}

void
XilImageFormat::setColorspace(XilColorspace* cspace)
{
    if(isTemp()) {
        //
        //  If it's a temporary image and has been written into (i.e. it's
        //  in the valid state), then it cannot be changed.
        //
        if(isValid()) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_USER, "di-428",
                          TRUE, this);
            return;
        }
    }

    //
    //  We need to sync() the object because if an operation used the image
    //  with a particular colorspace and the user changes that, deferred
    //  execution would cause an operation to be executed with the wrong
    //  colorspace.  BugID# 4028293.
    //
    allSync();

    //
    //  Update the version number before changing the object.
    //
    newVersion();

    if(cspace == NULL) {
        colorspace->destroy();
	colorspace = NULL;
    } else {
        if(cspace->getNBands() != nBands) {
            //
            //  Number of bands mismatch
            //
            XIL_OBJ_ERROR(getSystemState(),
                          XIL_ERROR_USER, "di-295", TRUE, cspace);
            return;
        }
        
	XilColorspace* new_colorspace = (XilColorspace*)cspace->createCopy();
        if(new_colorspace==NULL) {
            XIL_OBJ_ERROR(getSystemState(),
                          XIL_ERROR_RESOURCE, "di-1", TRUE, this);
            return;
        }
        
        colorspace->destroy();
        colorspace = new_colorspace;
    }
}

    
