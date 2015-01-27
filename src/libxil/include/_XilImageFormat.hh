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
//  File:	_XilImageFormat.hh
//  Project:	XIL
//  Revision:	1.21
//  Last Mod:	10:21:51, 03/10/00
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
//  MT Level:   UNSAFE
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilImageFormat.hh	1.21\t00/03/10  "

#ifndef _XIL_IMAGE_FORMAT_HH
#define _XIL_IMAGE_FORMAT_HH

#include "_XilDeferrableObject.hh"

class XilImageFormat : public XilDeferrableObject {
public:
    //
    //  The width (extent in x) of the image
    //
    unsigned int       getWidth();

    //
    //  The height (extent in y) of the image
    //
    unsigned int       getHeight();

    //  Synarx pixel dimensions
    float              getPixelWidth();
    float              getPixelHeight();

    void               setPixelWidth(float width);
    void               setPixelHeight(float height);

    //
    //  Returns the x and y image size
    //
    void               getSize(unsigned int* width,
                               unsigned int* height);

    //
    //  The number of bands in the image
    //
    unsigned int       getNumBands();

    //
    //  The data type of the image
    //
    XilDataType        getDataType();

    //
    //  Returns all of the image parameters
    //
    void               getInfo(unsigned int* width,
                               unsigned int* height, 
                               unsigned int* nbands,
                               XilDataType*  datatype);

    //
    //  Routines to set or get a copy of the XilImageFormat's colorspace
    //
    void	       setColorspace(XilColorspace* colorspace);

    //
    // Return a colorspace copy
    //
    XilColorspace*     getColorspace();

    //
    // Return an colorspace reference (pointer to the object)
    //
    XilColorspace*     refColorspace();

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
  
#include "XilImageFormatPrivate.hh"
  
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                       ~XilImageFormat();
#endif
};

#endif // _XIL_IMAGE_FORMAT_HH
